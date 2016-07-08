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

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_aes.h"
#include "b_playback_ip_utils.h"
#include "b_playback_ip_lm_helper.h"

#include <sys/ioctl.h>
#include <net/if.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include "bmp4_probe.h"
#include "bmp4_track.h"

#define MPEG_DASH_UNIT_TEST 0  /* Define this as non-zero to enable printing of stream information for unit testing.  */


/* Overall flow
   -Browser obtains the URI of a AV stream by parsing the html page (video tag for html5)
    -URI points to an MPD (Media Presentation Description) file which contains an ordered list of media URIs & informational tags
    -Each media URI refers to a media file which is a segment of of a single continous stream
*/
/* XML tags */
#define XML_TAG_MPD                         "MPD"
#define XML_TAG_LOCATION                    "Location"               /* Child of MPD */
#define XML_TAG_PERIOD                      "Period"                 /* Child of MPD */
#define XML_TAG_ADAPTATIONSET               "AdaptationSet"          /* Child of Period */
#define XML_TAG_CONTENTCOMPONENT            "ContentComponent"       /* Child of AdaptationSet */
#define XML_TAG_REPRESENTATION              "Representation"         /* Child of AdaptataionSet */
#define XML_TAG_SEGMENTLIST                 "SegmentList"            /* Child of Period, AdaptationSet, Representation */
#define XML_TAG_SEGMENTURL                  "SegmentURL"             /* Child of SegmentList */
#define XML_TAG_SEGMENTBASE                 "SegmentBase"            /* Child of Period, AdaptationSet, Representation */
#define XML_TAG_SEGMENTTEMPLATE             "SegmentTemplate"        /* Child of Period, AdaptationSet, Representation */
#define XML_TAG_BASEURL                     "BaseURL"                /* Child of MPD, Period, AdaptationSet, Representation */
#define XML_TAG_INITIALIZATIONSEGMENT       "Initialization"         /* Initialization segment is child of SegmentBase */
#define XML_TAG_REPRESENTATIONINDEX         "RepresentationIndex"    /* Initialization segment is child of SegmentBase */
#define XML_TAG_BITSTREAMSWITCHING          "BitstreamSwitching"     /* Child of MultipleSegmentBase */
#define XML_TAG_SEGMENTTIMELINE             "SegmentTimeline"        /* Child of MultipleSegmentBase */
#define XML_TAG_SEGMENTTIMELINEELEM         "S"                      /* Child of SegmentTimeliine */


/* XML attributes */

/* For MPDType */
#define XML_ATT_ID                                 "id"
#define XML_ATT_PROFILES                           "profiles"
#define XML_ATT_TYPE                               "type"
#define XML_ATT_AVAILABILITYSTARTTIME              "availabilityStartTime"
#define XML_ATT_AVAILABILITYENDTIME                "availabilityEndTime"
#define XML_ATT_MEDIAPRESENTATIONDURATION          "mediaPresentationDuration"
#define XML_ATT_MINIMUMUPDATEPERIOD                "minimumUpdatePeriod"
#define XML_ATT_MINBUFFERTIME                      "minBufferBime"
#define XML_ATT_TIMESHIFTBUFFERDEPTH               "timeShiftBufferDepth"
#define XML_ATT_SUGGESTEDPRESENTATIONDELAY         "suggestedPresentationDelay"
#define XML_ATT_MAXSEGMENTDURATION                 "maxSegmentDuration"
#define XML_ATT_MAXSUBSEGMENTDURATION              "maxSubsegmentDuration"

/* For PeriodType */
#define XML_ATT_HREF                               "href"
#define XML_ATT_ACTUATE                            "actuate"
#define XML_ATT_ID                                 "id"
#define XML_ATT_START                              "start"
#define XML_ATT_DURATION                           "duration"
#define XML_ATT_BITSTREAMSWITCHING                 "bitstreamSwitching"

/* For AdaptationSet */
#define XML_ATT_HREF                               "href"
#define XML_ATT_ACTUATE                            "actuate"
#define XML_ATT_ID                                 "id"
#define XML_ATT_GROUP                              "group"
#define XML_ATT_LANG                               "lang"
#define XML_ATT_CONTENTTYPE                        "contentType"
#define XML_ATT_PAR                                "par"
#define XML_ATT_MINBANDWIDTH                       "minBandwidth"
#define XML_ATT_MAXBANDWIDTH                       "maxBandwidth"
#define XML_ATT_MINWIDTH                           "minWidth"
#define XML_ATT_MAXWIDTH                           "maxWidth"
#define XML_ATT_MINHEIGHT                          "minHeight"
#define XML_ATT_MAXHEIGHT                          "maxHeight"
#define XML_ATT_MINFRAMERATE                       "minFrameRate"
#define XML_ATT_MAXFRAMERATE                       "maxFrameRate"
#define XML_ATT_SEGMENTALIGNMENT                   "segmentAlignment"
#define XML_ATT_SUBSEGMENTALIGNMENT                "subsegmentAlignment"
#define XML_ATT_SUBSEGMENTSTARTSWITHSAP            "subsegmentStartsWithSAP"
#define XML_ATT_BITSTREAMSWITCHING                 "bitstreamSwitching"


/* For RepresentationType */
#define XML_ATT_QUALITYRANKING                     "qualityRanking"
#define XML_ATT_DEPENDENCYID                       "dependencyId"
#define XML_ATT_MEDIASTREAMSTRUCTUREID             "mediaStreamStructureId"

/* For RepresentationBaseType */
#define XML_ATT_PROFILES                           "profiles"
#define XML_ATT_WIDTH                              "width"
#define XML_ATT_HEIGHT                             "height"
#define XML_ATT_SAR                                "sar"
#define XML_ATT_FRAMERATE                          "frameRate"
#define XML_ATT_AUDIOSAMPLINGRATE                  "audioSamplingRate"
#define XML_ATT_MIMETYPE                           "mimeType"
#define XML_ATT_SEGMENTPROFILES                    "segmentProfiles"
#define XML_ATT_CODECS                             "codecs"
#define XML_ATT_MAXIMUMSAPPERIOD                   "maximumSAPPeriod"
#define XML_ATT_STARTWITHSAP                       "startWithSAP"
#define XML_ATT_MAXPLAYOUTRATE                     "maxPlayoutRate"
#define XML_ATT_CODINGDEPENDENCY                   "codingDependency"
#define XML_ATT_SCANTYPE                           "scanType"

/* For MultipleSegmentBase */

#define XML_ATT_DURATION                          "duration"
#define XML_ATT_STARTNUMBER                       "startNumber"

/* For SegmentBase */
#define XML_ATT_TIMESCALE                         "timescale"
#define XML_ATT_PRESENTATIONTIMEOFFSET            "presentationTimeOffset"
#define XML_ATT_INDEXRANGE                        "indexRange"
#define XML_ATT_INDEXRANGEEXACT                   "indexRangeExact"

/* For SegmentURL */
#define XML_ATT_MEDIA                             "media"
#define XML_ATT_MEDIARANGE                        "mediaRange"
#define XML_ATT_INDEX                             "index"
#define XML_ATT_INDEXRANGE                        "indexRange"

/* For SegmentTemplate */
#define XML_ATT_INITIALIZATION                    "initialization"

/* For SegmentTimeline (SegmentTimelineElem) */
#define XML_ATT_SEGMENTTIMELINE_T                 "t"
#define XML_ATT_SEGMENTTIMELINE_D                 "d"
#define XML_ATT_SEGMENTTIMELINE_R                 "r"

/* For others */
#define XML_ATT_BANDWIDTH                         "bandwidth"
#define XML_ATT_MEDIA                             "media"
#define XML_ATT_TYPE                              "type"
#define XML_ATT_SOURCEURL                         "sourceURL"
#define XML_ATT_SERVICELOCATION                   "serviceLocation"
#define XML_ATT_BYTERANGE                         "byteRange"
#define XML_ATT_RANGE                             "range"


/* xml attribute values */
#define XML_VAL_CONTENT_TYPE_AUDIO                "audio"
#define XML_VAL_CONTENT_TYPE_VIDEO                "video"


#define BDBG_NUL(x)      /*  Use this to easily disable individual BDBG prints.*/

BDBG_MODULE(b_playback_ip_mpeg_dash);

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_summary);
#define PRINTMSG_SUMMARY(bdbg_args)             BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_summary, bdbg_args);

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_xml);
#define PRINTMSG_XML(bdbg_args)                 BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_xml, bdbg_args);

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_objects);
#define PRINTMSG_OBJECTS(bdbg_args)             BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_objects, bdbg_args);

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_session);
#define PRINTMSG_SESSION(bdbg_args)             BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_session, bdbg_args);

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_url);
#define PRINTMSG_URL(bdbg_args)                 BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_url, bdbg_args);

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_probe);
#define PRINTMSG_PROBE(bdbg_args)               BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_probe, bdbg_args);
#define PRINTMSG_PROBE_DUMP(hdr, prefix, pBuf, len)  PRINTMSG_MODULE_DUMP(BDBG_MODULE_MSG, b_playback_ip_mpeg_dash_probe, hdr, prefix, pBuf, len)

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_segbuf_read_bfile);
#define PRINTMSG_SEGBUF_READ(bdbg_args)         BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_segbuf_read_bfile, bdbg_args);

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_segbuf_write_bfile);
#define PRINTMSG_SEGBUF_WRITE(bdbg_args)        BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_segbuf_write_bfile, bdbg_args);

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_seek);
#define PRINTMSG_SEEK(bdbg_args)                BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_seek, bdbg_args);

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_download);
#define PRINTMSG_DOWNLOAD(bdbg_args)            BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_download, bdbg_args);

BDBG_FILE_MODULE(b_playback_ip_mpeg_dash_playback);
#define PRINTMSG_PLAYBACK(bdbg_args)            BDBG_MODULE_MSG(b_playback_ip_mpeg_dash_playback, bdbg_args);


bool  g_xmlDebugEnabled = false;     /* This will get set to true if the "b_playback_ip_mpeg_dash_xml" debug level is enabled. */
#define PRINT_XML_DEBUG(bdbg_args, node)  do { if (g_xmlDebugEnabled){BDBG_LOG( bdbg_args);  B_PlaybackIp_XmlElem_Print(node, 0);} } while(0);


#define MPEG_DASH_MPD_FILE_SIZE (500*1024)  /* Maximum MPD file size in bytes. */
#define MPEG_DASH_EVENT_TIMEOUT_MSEC 1000 /* 1 secs */
#define MPEG_DASH_READ_CHUNK_SIZE (TS_PKT_SIZE*HTTP_AES_BLOCK_SIZE*5)


#define PRINTMSG_MODULE_DUMP(bdbg_log_macro, bdbg_log_module, hdr, prefix, pBuf, len) \
do \
{ \
    bool  dbug_enabled = false; \
    bdbg_log_macro(bdbg_log_module, ("########## " hdr " ##########"));  /* Make sure that the module is registered with BDBG   */ \
    bdbg_log_macro(bdbg_log_module,("########## Start of dump of buffer: " #pBuf " ##########%s", dbug_enabled++? "":""));  /* Set dbug_enabled if the module is enabled           */ \
    if (dbug_enabled) \
    { \
        B_PlaybackIp_MpegDashDumpBuffer(prefix, pBuf, len); \
        bdbg_log_macro(bdbg_log_module,("########## End of dump. ##########")); \
    } \
} while (0)


/* Find a Representation's MPEG-DASH segment type (SegmentBase, SegmentList, SegmentTemplate) by first
 * looking in the Representation, then in the AdaptationSet, then in the Period. */
#define MPEG_DASH_GET_INHERITED_SEGMENT_TYPE(representationInfo, member)                                \
                  representationInfo->member               ? representationInfo->member :               \
                  representationInfo->parentObj->member    ? representationInfo->parentObj->member :    \
                  representationInfo->parentObj->parentObj->member


/* Find the value of a Representation's inherited segment property by first
 * looking in the Representation, then in the AdaptationSet, then in the Period. */
#define MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentType, member)                                        \
                  representationInfo->segmentType            && representationInfo->segmentType->member  ?                   \
                                                            representationInfo->segmentType->member :                        \
                  representationInfo->parentObj->segmentType && representationInfo->parentObj->segmentType->member  ?        \
                                                            representationInfo->parentObj->segmentType->member :             \
                  representationInfo->parentObj->parentObj->segmentType  ?                                                   \
                                                            representationInfo->parentObj->parentObj->segmentType->member :  \
                                                            0

/* Define an enum used to specify each of the five kinds of URLs. */
typedef enum {
    MPEG_DASH_URL_KIND_INITIALIZATION,      /* For initialization segment */
    MPEG_DASH_URL_KIND_BITSTREAMSWITCHING,  /* For bitstream switching segment */
    MPEG_DASH_URL_KIND_REPRESENTATIONINDEX, /* for representation index */
    MPEG_DASH_URL_KIND_SEGMENTINDEX,        /* for segment index */
    MPEG_DASH_URL_KIND_MEDIA                /* for media segment */
} MpegDashUrlKind;


extern int playback_ip_read_socket( B_PlaybackIpHandle playback_ip, void *securityHandle, int fd, char *buf, int buf_size, int timeout);
bool http_absolute_uri(char *url);
int B_PlaybackIp_SecuritySessionOpen( B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionOpenSettings *openSettings, int fd, void **newSecurityHandle);
NEXUS_TransportType http_get_payload_content_type(char *http_hdr);
extern int http_read_response(B_PlaybackIpHandle playback_ip, void *securityHandle, int fd, char **rbufp, int rbuf_size, char **http_hdr, char **http_payload, int *payload_len);
extern http_url_type_t http_get_url_type(char *http_hdr, char *url);

/* static void B_PlaybackIp_MpegDashInsertRepresentationInfoEntry(MpegDashSessionState *mpegDashSession, MpegDashRepresentationInfo *newRepresentationInfo); */

/* Local Prototypes */
static void B_PlaybackIp_MpegDashDestroyMpd(MpegDashMpdInfo *mpdInfo);
static void B_PlaybackIp_MpegDashDestroyPeriod(MpegDashPeriodInfo *periodInfo);
static void B_PlaybackIp_MpegDashDestroyAdaptationSet(MpegDashAdaptationSetInfo *adaptationSetInfo);
static void B_PlaybackIp_MpegDashDestroyContentComponent(MpegDashContentComponentInfo *contentComponentInfo);
static void B_PlaybackIp_MpegDashDestroyRepresentation(MpegDashRepresentationInfo *representationInfo);
static void B_PlaybackIp_MpegDashDestroySegmentBase(MpegDashSegmentBaseInfo *segmentBaseInfo);
static void B_PlaybackIp_MpegDashDestroyMultipleSegmentBase(MpegDashMultipleSegmentBaseInfo *multipleSegmentBaseInfo);
static void B_PlaybackIp_MpegDashDestroySegmentList(MpegDashSegmentListInfo *segmentListInfo);
static void B_PlaybackIp_MpegDashDestroySegmentTemplate(MpegDashSegmentTemplateInfo *segmentTemplateInfo);


int  B_PlaybackIp_MpegDashDumpBuffer(const char * msg, const void * dumpBuf, size_t len)
{
    unsigned char  * buf = (unsigned char *) dumpBuf;   /* Incomming buffer to be dumped. */
    char             fmtBuf[256];

    unsigned         fmtIdx;                            /* which byte in fmtBuf */
    unsigned         lineIdx;                           /* which value on the line */
    unsigned         bufIdx;                            /* index into buf */

    const unsigned   bytesPerLine = 16;

    for (bufIdx=0 ; bufIdx<len ; bufIdx+=bytesPerLine)
    {
        fmtIdx = 0;
        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "%s: ", msg);
        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, " 0x%04x: ", bufIdx);

        lineIdx = 0;
        while (lineIdx<16 )
        {
            if (lineIdx == 8)
            {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "  ");
            }

            if (bufIdx+lineIdx < len)
            {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, " %02x", buf[bufIdx+lineIdx]);
            }
            else
            {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "   ");
            }
            lineIdx++;
        }

        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "   |");

        lineIdx = 0;
        while (lineIdx<16 )
        {
            if (bufIdx+lineIdx < len)
            {
                unsigned char   byteToPrint = buf[bufIdx+lineIdx];
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "%c", isprint(byteToPrint)?byteToPrint:'.');
            }
            else
            {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, " ");
            }
            lineIdx++;
        }

        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "|");

        BDBG_LOG(("%s", fmtBuf));
    }
    return 0;
}

/*****************************************************************************
 *  This function is analogous to the C Library's mktime() function, but
 *  instead of interpreting the "struct tm" as local time, it is interpreted
 *  as UTC time.  So just as mktime() is the inverse of localtime(), this
 *  function is the inverse of gmtime().
 *****************************************************************************/
static time_t B_PlaybackIp_MpegDashMakeGmTime(struct tm  * brokenDownTime)
{
    long        tzOffsetInSeconds;
    time_t      localTime;
    time_t      gmTime;

    struct tm   originalBrokenDownTime;
    struct tm   adjustedBrokenDownTime;


    /*------------------------------------------------------------------------
     *  Make a copy of the passed tm struct because mktime() may change it.
     *------------------------------------------------------------------------*/
    originalBrokenDownTime = *brokenDownTime;
    memset (&adjustedBrokenDownTime, 0, sizeof (adjustedBrokenDownTime));

    /*------------------------------------------------------------------------
     *  Now take the broken down time (struct tm) and convert it to a time_t
     *  (seconds since 1970) that represents UTC time. But we can't do that
     *  directly,  Start by converting the broken down time to a time_t
     *  using mktime() (but mktime() will interpret the broken down time as
     *  being in the local timezone... so we'll need to compensate for that
     *  later).
     *--------------------------------------------------------------------*/
    localTime = mktime(&originalBrokenDownTime);
    if (localTime == -1) goto error;

    /*--------------------------------------------------------------------
     *  Now the resulting time_t back to a broken down time (struct
     *  tm) that represents UTC time (instead of local time) by using
     *  the gmtime_r() runction.   The resulting broken down time
     *  will differ from the original broken down time by an amount
     *  that indicates the time zone difference between local time
     *  and UTC time.
     *--------------------------------------------------------------------*/
    gmtime_r(&localTime, &adjustedBrokenDownTime);

    /*--------------------------------------------------------------------
     *  Then convert the UTC flavor of broken down time into a time_t.
     *--------------------------------------------------------------------*/
    gmTime = mktime(&adjustedBrokenDownTime);
    if (gmTime == -1) goto error;

    /*--------------------------------------------------------------------
     *  Okay, now the two time_t's (localTime and gmTime) should differ by
     *  the local timezone difference from UTC.  Use difftime to find the
     *  number of seconds between the two.
     *--------------------------------------------------------------------*/
    tzOffsetInSeconds = (int) difftime( localTime, gmTime);

    /*--------------------------------------------------------------------
     *  Now add in our local time offset to the original broken down time so
     *  that it will undo the local time conversion that mktime() does.
     *--------------------------------------------------------------------*/
    originalBrokenDownTime.tm_sec += tzOffsetInSeconds;

    /*--------------------------------------------------------------------
     *  Finally convert to a time_t to get the time as UTC time.
     *--------------------------------------------------------------------*/
    gmTime = mktime(&originalBrokenDownTime);
    if (gmTime == -1) goto error;

    return gmTime;

error:
    return -1;
}


/*****************************************************************************
 *  Given a time duration as a 64-bit integer number of milliseconds, this
 *  function will convert it to the PlaybackIp_MpegDashDuration data type.
 *****************************************************************************/
bool B_PlaybackIp_MpegDashGetDurationFromMs(uint64_t durationInMs, MpegDashDuration  * duration)
{
    memset(duration, 0, sizeof *duration);

    duration->valid = true;
    duration->nsec = (durationInMs % 1000) * 1000000000;
    duration->sec  = durationInMs / 1000;

    return true;
}


/*****************************************************************************
 *  Given a time duration as a PlaybackIp_MpegDashDuration data type, this
 *  function will convert it to a 64-bit integer number of milliseconds.
 *****************************************************************************/
bool B_PlaybackIp_MpegDashGetMsFromDuration(MpegDashDuration  * duration,  uint64_t  * durationInMs)
{
    uint64_t    myDurationInMs;

    if (!duration->valid) goto error;
    if (duration->minusSign) goto error;
    if (duration->year != 0) goto error;
    if (duration->mon != 0) goto error;

    myDurationInMs = duration->nsec / 1000000;
    myDurationInMs = duration->mday;  /* days */
    myDurationInMs = myDurationInMs * 24 + duration->hour; /* hours */
    myDurationInMs = myDurationInMs * 60 + duration->min;
    myDurationInMs = myDurationInMs * 60 + duration->sec;
    myDurationInMs = myDurationInMs * 1000 + (duration->nsec/1000000);

    *durationInMs = myDurationInMs;
    return true;

error:
    return false;
}


/*****************************************************************************
 *  This function changes the sign of a PlaybackIp_MpegDashDuration
 *  data type, changing between positive and negative durations (and
 *  vice-versa).
 *****************************************************************************/
bool B_PlaybackIp_MpegDashGetDurationComplement(MpegDashDuration  * duration)
{
    if (!duration->valid) goto error;

    duration->minusSign = ! duration->minusSign;
    return true;

error:
    return false;
}


/*****************************************************************************
 *  This function changes the sign of a PlaybackIp_MpegDashDuration
 *  data type, changing between positive and negative durations (and
 *  vice-versa).
 *****************************************************************************/
bool B_PlaybackIp_MpegDashGetDateTimePlusDuration(MpegDashDateTime  *xmlDateTime, MpegDashDuration  * xmlDuration)
{
    MpegDashDateTime  myDateTime;
    struct tm               myBrokenDownTime;
    long                    signValue = 1;  /* assume positive duration.  */

    if (!xmlDateTime->valid) goto error;
    if (!xmlDuration->valid) goto error;

    myDateTime = *xmlDateTime;

    gmtime_r(&xmlDateTime->utc_time_t, &myBrokenDownTime);

    if (xmlDuration->minusSign) {
        signValue = -1;
    }

    myBrokenDownTime.tm_year +=  signValue * xmlDuration->year;
    myBrokenDownTime.tm_mon  +=  signValue * xmlDuration->mon;
    myBrokenDownTime.tm_mday +=  signValue * xmlDuration->mday;
    myBrokenDownTime.tm_hour +=  signValue * xmlDuration->hour;
    myBrokenDownTime.tm_min  +=  signValue * xmlDuration->min;
    myBrokenDownTime.tm_sec  +=  signValue * xmlDuration->sec;

    myDateTime.nsecs = xmlDateTime->nsecs + (signValue * xmlDuration->nsec);
    if (myDateTime.nsecs > 1000000000) {
        myDateTime.nsecs -= 1000000000;
        myBrokenDownTime.tm_sec++;
    }

    myDateTime.utc_time_t = B_PlaybackIp_MpegDashMakeGmTime(&myBrokenDownTime);
    if (myDateTime.utc_time_t == -1) goto error;

    *xmlDateTime = myDateTime;

    return true;

error:
    return false;
}


/*****************************************************************************
 *  This function subtracts two PlaybackIp_MpegDashDateTime data
 *  types and returns the difference as a
 *  PlaybackIp_MpegDashDuration data type.
 *****************************************************************************/
static void B_PlaybackIp_MpegDashGetDateTimeDifference(  const MpegDashDateTime  * minuend,
                                                         const MpegDashDateTime  * subtrahend,
                                                               MpegDashDuration  * difference)
{
    int                 borrowFlag = 0;

    memset(difference, 0, sizeof *difference);

    difference->nsec = minuend->nsecs - subtrahend->nsecs;
    if (difference->nsec >= 0)
    {
        borrowFlag = 0;
    }
    else
    {
        difference->nsec += 1000000000;
        borrowFlag = 1;
    }

    difference->sec = (int) difftime( minuend->utc_time_t, subtrahend->utc_time_t) - borrowFlag;

    if (difference->sec < 0 || (difference->sec == 0 && difference->nsec < 0))
    {
        difference->minusSign = true;
        difference->nsec = subtrahend->nsecs - minuend->nsecs;
        if (difference->nsec >= 0)
        {
            borrowFlag = 0;
        }
        else
        {
            difference->nsec += 1000000000;
            borrowFlag = true;
        }
        difference->sec = (int) difftime( subtrahend->utc_time_t, minuend->utc_time_t) - borrowFlag;
    }
    difference->valid = true;
}


/*****************************************************************************
 *  This function parses a "duration" attribute from an XML file and stores
 *  the result into a  PlaybackIp_MpegDashDuration data type.
 *****************************************************************************/
bool B_PlaybackIp_MpegDashGetDurationFromXml(const char * buf, MpegDashDuration  * xmlDuration)
{
    const char * pCh = buf;
    int  number;
    MpegDashDuration  myXmlDuration;

    char * p;  /* Workaround for strtol() const-ness issue. */

    /*------------------------------------------------------------------------
     *  The dateTime format is specified here:
     *  http://www.w3.org/TR/xmlschema-2/#duration and looks like this:
     *  "PnYnMnDTnHnMnS". So parse each of the fields into a
     *  "PlaybackIp_MpegDashDuration" struct (our own broken down time
     *  structure).
     *------------------------------------------------------------------------*/

    memset(&myXmlDuration, 0, sizeof myXmlDuration);

    /*------------------------------------------------------------------------
     *  Get past any preceeding white space.
     *------------------------------------------------------------------------*/
    while (isspace(*pCh)) pCh++;

    /*------------------------------------------------------------------------
     *  Look for the leading minus sign.
     *------------------------------------------------------------------------*/
    if (*pCh == '-') {
        pCh++;
        myXmlDuration.minusSign = true;
    }

    /*------------------------------------------------------------------------
     *  Now there needs to be a leading 'P'
     *------------------------------------------------------------------------*/
    if (*pCh++ != 'P') goto error;

    /*------------------------------------------------------------------------
     *  Then comes a number, but we don't know what it represents yet, so
     *  just convert it to an integer.
     *------------------------------------------------------------------------*/
    number = strtol( pCh, &p, 10);  pCh = p;

    if (*pCh == 'Y')    /* Years */
    {
        myXmlDuration.year = number;
        pCh++;
        number = strtol( pCh, &p, 10);  pCh = p;
    }

    if (*pCh == 'M')    /* Months */
    {
        myXmlDuration.mon = number;
        pCh++;
        number = strtol( pCh, &p, 10);  pCh = p;
    }

    if (*pCh == 'D')    /* Days */
    {
        myXmlDuration.mday = number;
        pCh++;
        number = strtol( pCh, &p, 10);  pCh = p;
    }

    /*------------------------------------------------------------------------
     *  Now there might be a 'T' if there is any time (hours, minutes,
     *  seconds) component to the duration.  But if no time component, then
     *  the 'T' can be left out.
     *------------------------------------------------------------------------*/
    if (*pCh == 'T') {
        pCh++;

        /*--------------------------------------------------------------------
         *  There shouldn't have been any number immediately before the 'T'.
         *--------------------------------------------------------------------*/
        if (number != 0) goto error;

        /*--------------------------------------------------------------------
         *  But there might be a number after it.
         *--------------------------------------------------------------------*/
        number = strtol( pCh, &p, 10);  pCh = p;

        if (*pCh == 'H')    /* Hours */
        {
            myXmlDuration.hour = number;
            pCh++;
            number = strtol( pCh, &p, 10);  pCh = p;
        }

        if (*pCh == 'M')    /* Minutes */
        {
            myXmlDuration.min = number;
            pCh++;
            number = strtol( pCh, &p, 10);  pCh = p;
        }

        /*--------------------------------------------------------------------
         *  If we come to a '.', then what follows is the fractional part of
         *  the Seconds (and number contains the integer part).
         *  The fractional seconds are represented as a number of
         *  nanoseconds.  So start accumulating the number of nanoseconds
         *  represented by each digit after the decimal.
         *--------------------------------------------------------------------*/
        if (*pCh == '.')
        {
            unsigned long  nanoseconds = 100000000;
            unsigned long  digit;

            pCh++;  /* Move past the period. */

            while(isdigit(*pCh)) {
                digit = *pCh++ - '0';
                myXmlDuration.nsec += digit * nanoseconds;
                nanoseconds /= 10;
            }

            if (*pCh != 'S') goto error;
        }

        /*--------------------------------------------------------------------
         *  If there's an 'S', then the current number is the integer part of
         *  the Seconds.
         *--------------------------------------------------------------------*/
        if (*pCh == 'S') {
            myXmlDuration.sec = number;
            pCh++;
        }
    }

    /*------------------------------------------------------------------------
     *  Allow for trailing whitespace (though not really required).
     *------------------------------------------------------------------------*/
    while (*pCh != '\0') {
        if (!isspace(pCh)) goto error;
        pCh++;
    }

    /*--------------------------------------------------------------------
     *  Then just copy the results into the caller's structure.
     *--------------------------------------------------------------------*/
    myXmlDuration.valid = true;
    *xmlDuration = myXmlDuration;

    return 0;

error:
    return -1;
}


/*****************************************************************************
 *  This function parses a "dateTime" attribute from an XML file
 *  and stores the result into a  PlaybackIp_MpegDashDateTime data type.
 *****************************************************************************/
bool B_PlaybackIp_MpegDashGetDateTimeFromXml(const char * buf, MpegDashDateTime  * xmlTime)
{
    const char * pCh = buf;
    char       * p;  /* Workaround for strtol() const-ness issue. */
    struct tm      originalBrokenDownTime;
    time_t         gmTime;
    unsigned long  nanoseconds = 0;
    long           timezoneSeconds = 0;


    memset (&originalBrokenDownTime, 0, sizeof (originalBrokenDownTime));

    /*------------------------------------------------------------------------
     *  The dateTime format is specified here:
     *  http://www.w3.org/TR/xmlschema-2/#duration and looks like this:
     *   '-'? yyyy '-' mm '-' dd 'T' hh ':' mm ':' ss ('.' s+)? (zzzzzz)?
     *   So parse each of the fields into a "struct tm" (the Linux broken down
     *   time structure).
     *------------------------------------------------------------------------*/

    originalBrokenDownTime.tm_year = strtol( pCh, &p, 10);  pCh = p;
    originalBrokenDownTime.tm_year -= 1900;
    if (*pCh++ != '-') goto error;

    originalBrokenDownTime.tm_mon = strtol( pCh, &p, 10);  pCh = p;
    originalBrokenDownTime.tm_mon -= 1; /* tm_mon range is [0 - 11] */
    if (*pCh++ != '-') goto error;

    originalBrokenDownTime.tm_mday = strtol( pCh, &p, 10);  pCh = p;
    if (*pCh++ != 'T') goto error;

    originalBrokenDownTime.tm_hour = strtol( pCh, &p, 10);  pCh = p;
    if (*pCh++ != ':') goto error;

    originalBrokenDownTime.tm_min = strtol( pCh, &p, 10);  pCh = p;
    if (*pCh++ != ':') goto error;

    originalBrokenDownTime.tm_sec = strtol( pCh, &p, 10);  pCh = p;

    /*------------------------------------------------------------------------
     *  For the fractional seconds, we represent them as a number of
     *  nanoseconds.  So start accumulating the number of nanoseconds
     *  represented by each digit after the decimal.
     *------------------------------------------------------------------------*/
    if (*pCh == '.') {
        unsigned long  nsecsPerDigit = 100000000;
        unsigned long  digit;

        pCh++;
        while(isdigit(*pCh)) {
            digit = *pCh++ - '0';
            nanoseconds += digit * nsecsPerDigit;
            nsecsPerDigit /= 10;
        }
    }

    /*------------------------------------------------------------------------
     *  If a timezone is included, convert it to a number of seconds
     *  offset from UTC.
     *------------------------------------------------------------------------*/
    if (*pCh == 'Z') {      /* 'Z' => UTC timezone */
        pCh++;
    }
    else if (*pCh == '+' || *pCh == '-') {
        bool minusSign = (*pCh++ == '-') ? true : false;

        timezoneSeconds = strtol( pCh, &p, 10) * 3600;  pCh = p;
        if (*pCh++ != ':') goto error;

        timezoneSeconds += strtol( pCh, &p, 10) * 60;  pCh = p;

        if (minusSign) timezoneSeconds = -timezoneSeconds;
    }

    /*--------------------------------------------------------------------
     *  Also adjust the original broken down time by any timezone adjustment
     *  that was part of the input string.  This will give us the time
     *  in UTC... but we'll still keep the timezone adjustment in case we
     *  need to convert it back to it's original timezone (which probably
     *  isn't our timezone).
     *--------------------------------------------------------------------*/
    originalBrokenDownTime.tm_sec -= timezoneSeconds;

    /*--------------------------------------------------------------------
     *  Finally convert to a time_t to get the time as UTC time.
     *--------------------------------------------------------------------*/
    gmTime = B_PlaybackIp_MpegDashMakeGmTime(&originalBrokenDownTime);
    if (gmTime == -1) goto error;


    /*--------------------------------------------------------------------
     *  Then just copy the results into the caller's structure.
     *--------------------------------------------------------------------*/
    xmlTime->valid = true;
    xmlTime->nsecs = nanoseconds;
    xmlTime->utc_time_t = gmTime;
    xmlTime->tz_secs = timezoneSeconds;

    return 0;

error:
    return -1;
}


/*****************************************************************************
 *  This function populates a PlaybackIp_MpegDashDateTime data type with
 *  the current UTC time.
 *****************************************************************************/
static void B_PlaybackIp_MpegDashGetDateTimeNow(MpegDashDateTime  *dateTime)
{
    MpegDashDateTime  myDateTime;
    struct timespec              myTimespec;

    clock_gettime(CLOCK_REALTIME, &myTimespec);

    myDateTime.utc_time_t = myTimespec.tv_sec;
    myDateTime.nsecs      = myTimespec.tv_nsec;
    myDateTime.tz_secs    = 0;      /* We'll just use UTC for "now". */
    myDateTime.valid      = true;

    *dateTime = myDateTime;         /* Copy to caller's structure. */
}


/*****************************************************************************
 * Reset the bandwidth accumulator.
 *****************************************************************************/
static void B_PlaybackIp_MpegDashResetBandwidthSample(MpegDashBandwidthContext  * bandwidthContext, unsigned long sampleLimit)
{
    /* Zero out everything... */
    BKNI_Memset(bandwidthContext, 0, sizeof (*bandwidthContext));

    /* Then set the sample limit as requested. */
    bandwidthContext->sampleLimit = sampleLimit;
}


/*****************************************************************************
 * Add a bandwidth sample to the bandwidth accumulator.
 *****************************************************************************/
static void B_PlaybackIp_MpegDashAddBandwidthSample(MpegDashBandwidthContext  * bandwidthContext, unsigned long timeInMs, unsigned long bytes)
{
    char        * myFuncName = "AddBwSample";

    if (bandwidthContext->sampleLimit == 0 ) bandwidthContext->sampleLimit = 1;

    if (bandwidthContext->accumulatedSamples < bandwidthContext->sampleLimit) {

        bandwidthContext->accumulatedSamples++;
    }
    else {

        bandwidthContext->accumulatedBytes    -= bandwidthContext->accumulatedBytes    / bandwidthContext->sampleLimit;
        bandwidthContext->accumulatedTimeInMs -= bandwidthContext->accumulatedTimeInMs / bandwidthContext->sampleLimit;
        bandwidthContext->accumulatedTimeWeightedKBps -= bandwidthContext->accumulatedTimeWeightedKBps / bandwidthContext->sampleLimit;
        bandwidthContext->accumulatedByteWeightedKBps -= bandwidthContext->accumulatedByteWeightedKBps / bandwidthContext->sampleLimit;
    }

    bandwidthContext->accumulatedTimeInMs += timeInMs;
    bandwidthContext->accumulatedBytes    += bytes;
    bandwidthContext->accumulatedTimeWeightedKBps += bytes;     /* actually: timeInMs * (bytes / timeInMs) */
    bandwidthContext->accumulatedByteWeightedKBps += bytes * (bytes / timeInMs);

#if 0  /* ==================== Original Algorithm (time weighted)  ==================== */
    bandwidthContext->filteredBandwidthInKbitsPerSecond = (bandwidthContext->accumulatedBytes / bandwidthContext->accumulatedTimeInMs) * 8;
#elif 1  /* ==================== Alternate Algorithm (time weighted)  ==================== */
    bandwidthContext->filteredBandwidthInKbitsPerSecond = (bandwidthContext->accumulatedTimeWeightedKBps / bandwidthContext->accumulatedTimeInMs) * 8;
#elif 0  /* ==================== Alternate Algorithm (bytecount weighted)  ==================== */
    bandwidthContext->filteredBandwidthInKbitsPerSecond = (bandwidthContext->accumulatedByteWeightedKBps / bandwidthContext->accumulatedTimeInMs) * 8;
#endif /* ==================== GARYWASHERE - End of Modified Code   ==================== */

    PRINTMSG_SUMMARY(("%s:%d : Adding BW sample: timeInMs: %lu, bytes: %lu Kbps: %lu Averaged Kbps: %lu",
                      myFuncName, __LINE__,
                      timeInMs,
                      bytes,
                      (bytes * 8) / timeInMs,
                      bandwidthContext->filteredBandwidthInKbitsPerSecond    ));
}


/*****************************************************************************
 * Get the current average bandwidth.
 *****************************************************************************/
static unsigned long B_PlaybackIp_MpegDashGetCurrentBandwidth(MpegDashBandwidthContext  * bandwidthContext)
{
    return bandwidthContext->filteredBandwidthInKbitsPerSecond;
}



/*****************************************************************************
 * Check the HTTP response header and the URL suffix to decide if the
 * request is for an MPEG-DASH session.
 *****************************************************************************/
bool
B_PlaybackIp_IsMpegDashSession (
    char *mpdUri,
    char *httpResponseHeader
    )
{
    char *contentType;
    /*
     *  Either file name or HTTP Content-type will indicate it be an MPEG-DASH MPD file.
     *  Assume DASH if file name ends w/ ".mpd" and/or HTTP Content-type is "application/dash+xml"
     */
    if (B_PlaybackIp_UtilsStristr(mpdUri, ".mpd") != NULL) {
        PRINTMSG_SESSION(("%s: MPEG-DASH MPD file URI: %s", __FUNCTION__, mpdUri));
        return true;
    }

    if ((contentType = B_PlaybackIp_UtilsStristr(httpResponseHeader, "Content-Type: ")) != NULL) {
        contentType += strlen("Content-Type: ");

        if (B_PlaybackIp_UtilsStristr(contentType, "application/dash+xml")) {
            PRINTMSG_SESSION(("%s: MPEG-DASH MPD file URI %s, contentType %s", __FUNCTION__, mpdUri, contentType));
            return true;
        }
    }

    /* Neither URI name nor content type indicates that the session is MPEG-DASH. */
    return false;
}


/*****************************************************************************
 * Try to find and allocate a SegmentBuffer for downloading the next
 * media (or initialization) segment.
 *****************************************************************************/
static MpegDashSegmentBuffer *
B_PlaybackIp_MpegDashSegmentBufferAlloc(B_PlaybackIpHandle playback_ip)
{
    MpegDashSessionState *mpegDashSession = playback_ip->mpegDashSessionState;
    MpegDashSegmentBuffer *segmentBuffer = NULL;
    int i;

    /* determine the next buffer to use for downloading next media segment */
    segmentBuffer = NULL;

    for (i=0; !segmentBuffer && i<MPEG_DASH_NUM_SEGMENT_BUFFERS; i++) {
        BKNI_AcquireMutex(mpegDashSession->segmentBuffer[i].lock);
        if (!mpegDashSession->segmentBuffer[i].allocated) {
            segmentBuffer = &mpegDashSession->segmentBuffer[i];

            segmentBuffer->allocated           = true;
            segmentBuffer->filled              = false;
            segmentBuffer->markedDiscontinuity = false;
            segmentBuffer->bufferDepth         = 0;
            segmentBuffer->mp4BoxPrefixSize    = 0;
        }
        BKNI_ReleaseMutex(mpegDashSession->segmentBuffer[i].lock);
    }

    return segmentBuffer;   /* NULL if no segment available */
}

/*****************************************************************************
 * Free up a SegmentBuffer after its contents have been processed and are
 * no longer needed.
 *****************************************************************************/
static void
B_PlaybackIp_MpegDashSegmentBufferFree(B_PlaybackIpHandle playback_ip, MpegDashSegmentBuffer *segmentBuffer)
{
    BSTD_UNUSED(playback_ip);

    segmentBuffer->allocated = false;
}

/*****************************************************************************
 * One-time initialization code to setup the SegmentBuffer data structures.
 *****************************************************************************/
static int
B_PlaybackIp_MpegDashSegmentBufferSetup(
    B_PlaybackIpHandle playback_ip
    )
{
    MpegDashSessionState *mpegDashSession = playback_ip->mpegDashSessionState;

    /* use index & data cache for downloading the consecutive media segments */

    mpegDashSession->segmentBuffer[0].allocated = false;
    mpegDashSession->segmentBuffer[0].filled    = false;
    mpegDashSession->segmentBuffer[0].buffer    = playback_ip->indexCache;
    mpegDashSession->segmentBuffer[0].bufferSize = playback_ip->indexCacheSize;
    mpegDashSession->segmentBuffer[0].bufferDepth = 0;
    if (BKNI_CreateMutex(&mpegDashSession->segmentBuffer[0].lock) != 0) {
        BDBG_ERR(("%s: Failed to create BKNI mutex at %d", __FUNCTION__, __LINE__));
        goto error;
    }
    PRINTMSG_SESSION(("%s: Using index cache (%u KB) as segment buffer # 0", __FUNCTION__, (unsigned)(mpegDashSession->segmentBuffer[0].bufferSize / 1000)));

    mpegDashSession->segmentBuffer[1].allocated = false;
    mpegDashSession->segmentBuffer[1].filled    = false;
    mpegDashSession->segmentBuffer[1].buffer    = playback_ip->dataCache[0].cache;
    mpegDashSession->segmentBuffer[1].bufferSize = playback_ip->dataCache[0].size;
    mpegDashSession->segmentBuffer[1].bufferDepth = 0;
    if (BKNI_CreateMutex(&mpegDashSession->segmentBuffer[1].lock) != 0) {
        BDBG_ERR(("%s: Failed to create BKNI mutex at %d", __FUNCTION__, __LINE__));
        goto error;
    }
    PRINTMSG_SESSION(("%s: Using data cache (%u KB) as segment buffer # 1", __FUNCTION__, (unsigned)(mpegDashSession->segmentBuffer[1].bufferSize / 1000)));

    return 0;
error:
    return -1;
}

/*****************************************************************************
 * Build an absolute URI (URL) from its components.
 *****************************************************************************/
static char *
B_PlaybackIp_MpegDashBuildAbsoluteUri(char *server, int port, char *baseUri, char *fileName)
{
    int uriLength;
    char portString[16];
    char *uri;
    char *tmp1, *tmp2 = NULL;
    int baseUriLength;
    char *baseUriCopy = NULL;
    int count;

    BDBG_ASSERT(server);
    BDBG_ASSERT(baseUri);
    BDBG_ASSERT(fileName);

    /* determine the # of char for the port */
    memset(portString, 0, sizeof(portString));
    snprintf(portString, sizeof(portString)-1, "%d", port);

    /* check if we need to use path from the base URI */
    tmp1 = baseUri;
    while ((tmp1 = strstr(tmp1, "/")) != NULL) {
        tmp2 = tmp1; /* note location of next directory path */
        tmp1 += 1; /* move past the / char */
    }
    if (tmp2) {
        baseUriLength = tmp2 - baseUri + 1 + 1; /* one for / char, one for NULL char */
        if ((baseUriCopy = (char *)BKNI_Malloc(baseUriLength)) == NULL) {
            BDBG_ERR(("%s: ERROR: failed to allocate %d bytes of memory at %d\n", __FUNCTION__, baseUriLength, __LINE__));
            return NULL;
        }
        strncpy(baseUriCopy, baseUri, baseUriLength-1);
        baseUriCopy[baseUriLength-1] = '\0';
    }
    else {
        /* no path to use from baseUri */
        baseUriLength = 0;
    }

    /* now allocate space for holding the absolute URI */
    uriLength = strlen(server) + strlen(portString) + baseUriLength + strlen(fileName) + 11; /* extra characters for http header */
    if ((uri = (char *)BKNI_Malloc(uriLength)) == NULL) {
        if (baseUriCopy)
            BKNI_Free(baseUriCopy);
        BDBG_ERR(("%s: Failed to allocate %d bytes of memory for building uri", __FUNCTION__, uriLength));
        return NULL;
    }
    count = snprintf(uri, uriLength, "http://%s:%s%s%s", server, portString, baseUriLength ? baseUriCopy : "", fileName);
    uri[uriLength-1] = '\0';
    PRINTMSG_URL(("%s: server %s port %s ", __FUNCTION__, server, portString));
    PRINTMSG_URL(("base uri %s", baseUriLength ? baseUri : ""));
    PRINTMSG_URL(("file %s", fileName));
    PRINTMSG_URL(("Absolute uri %s", uri));
    if (baseUriCopy)
        BKNI_Free(baseUriCopy);
    return uri;
}


/*****************************************************************************
 * Finish a download (which was started by B_PlaybackIp_HttpSessionOpen)
 * from the current network socket into the specified buffer and return the
 * amount of bytes read.  Read until EOF, error condition, or channel change
 * (state change) occurs.
 * More specifically, this function is used to finish downloading any
 * remaining portion of the MPD file.
 *****************************************************************************/
static bool
B_PlaybackIp_MpegDashFinishFileDownload(B_PlaybackIpHandle playback_ip, int fd, char *buffer, int bufferSize, int *totalBytesRead, bool nullTerminate)
{
    ssize_t bytesRead = 0;
    int bytesToRead;
    bool serverClosed = false;

    BDBG_ENTER(B_PlaybackIp_MpegDashFinishFileDownload);

    PRINTMSG_DOWNLOAD(("%s: Continuing download, currently read %d", __FUNCTION__, *totalBytesRead));
    while (true) {
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode)) {
            PRINTMSG_DOWNLOAD(("%s: breaking file download loop due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
            break;
        }

        if (playback_ip->contentLength > 0 && *totalBytesRead >= (int)playback_ip->contentLength) {
            /* we have read all the bytes that server had indicated via contentLength, so instead of trying another read and waiting for server to close the connection */
            /* consider this as server closed event and break out of the read loop */
            PRINTMSG_DOWNLOAD(("%s: breaking out of read loop as we have read %d upto the content length %lld", __FUNCTION__, *totalBytesRead, (long long)playback_ip->contentLength));
            serverClosed = true;
            break;
        }

        /* Figure out how many bytes to read. */
        if (playback_ip->contentLength > 0) {
            bytesToRead = playback_ip->contentLength;  /* Total bytes to read. */
            if (*totalBytesRead >= bytesToRead) {
                serverClosed = true;
                break;                                  /* Done reading all bytes from server. */
            }
        }
        else {
            bytesToRead = bufferSize;                   /* Total bytes to read. */
            if (*totalBytesRead >= bytesToRead) {
                break;                                  /* Caller's buffer is full. */
            }
        }

        if (bytesToRead > bufferSize) {
             bytesToRead = bufferSize;                  /* Limit to caller's buffer size. */
        }

        bytesToRead -= *totalBytesRead;                 /* Remaining bytes left to read. */

        if (bytesToRead > MPEG_DASH_READ_CHUNK_SIZE) {
             bytesToRead = MPEG_DASH_READ_CHUNK_SIZE;   /* Limit to our max chunk size. */
        }


        /* make sure there is enough space in the read buffer */
        if ((*totalBytesRead + bytesToRead) > bufferSize) {
            PRINTMSG_DOWNLOAD(("%s: need bigger buffer to hold the complete downloaded file: totalBytesRead %d, size %d, returning what is read", __FUNCTION__, *totalBytesRead, bufferSize));
            break;
        }

        if ((bytesRead = playback_ip_read_socket(playback_ip, playback_ip->securityHandle, fd, buffer+*totalBytesRead, bytesToRead, playback_ip->networkTimeout)) <= 0) {
            if (playback_ip->selectTimeout) {
                PRINTMSG_DOWNLOAD(("%s: socket error, retry read: size %d, errno :%d, state %d, select timeout %d, server closed %d",
                                   __FUNCTION__, (int)(*totalBytesRead+bytesRead), errno, playback_ip->playback_state, playback_ip->selectTimeout, playback_ip->serverClosed));
                continue;
            }
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: Network Read Error, rc %d, playback ip state %d", __FUNCTION__, (int)bytesRead, playback_ip->playback_state));
#endif
            serverClosed = true;
            break;
        }

        /* Received some data, increment the count. */
        *totalBytesRead += bytesRead;
    }
    PRINTMSG_DOWNLOAD(("%s: finished downloading file: errno %d, size %d, state %d, server closed %d on socket %d", __FUNCTION__, errno, *totalBytesRead, playback_ip->playback_state, serverClosed, fd));
    if (serverClosed) {
        /* close security context and socket */
        if (playback_ip->securityHandle) {
            playback_ip->netIo.close(playback_ip->securityHandle);
            playback_ip->securityHandle = NULL;
        }
        close(fd);
    }

    /* downloaded the file into the buffer, return success */
    if (*totalBytesRead) {
        if (*totalBytesRead < bufferSize) {
            if (nullTerminate)
                buffer[*totalBytesRead] = '\0'; /* null terminate the downloaded file */
        }
        else {
            BDBG_ERR(("%s: increase the file buffer size from %d, it is not big enough", __FUNCTION__, bufferSize));
            BDBG_LEAVE(B_PlaybackIp_MpegDashFinishFileDownload);
            return false;
        }
        BDBG_LEAVE(B_PlaybackIp_MpegDashFinishFileDownload);
        return true;
    }
    else
    {
        return false;
        BDBG_LEAVE(B_PlaybackIp_MpegDashFinishFileDownload);
    }
}

/*****************************************************************************
 * Finish a download (which was started by
 * B_PlaybackIp_MpegDashMediaSegmentDownloadStart) from the
 * current network socket into the specified buffer and return
 * the number of bytes read.  Read until EOF, error condition,
 * or channel change (state change) occurs.  In addition, also
 * measure the network bandwidth while downloading this data.
 * More specifically, this function is used to finish
 * downloading media segments and initialization segments.
 *****************************************************************************/
static bool
B_PlaybackIp_MpegDashMediaSegmentDownloadFinish(
    B_PlaybackIpHandle playback_ip,
    MpegDashDownloadContext *downloadContext,
    bool *serverClosed
    )
{
    ssize_t bytesRead = 0;
    int     bytesToRead = 0;
    bool    rc = true;      /* Assume success. */

    B_Time          beginTime, endTime;
    unsigned int    totalDownloadTime;
    ssize_t         totalBytesLeftToRead;
    MpegDashSegmentBuffer   *segmentBuffer = downloadContext->segmentBuffer;

    BDBG_ENTER(B_PlaybackIp_MpegDashMediaSegmentDownloadFinish);

    *serverClosed = false;

    totalBytesLeftToRead = downloadContext->totalBytesExpected - downloadContext->totalBytesReadSoFar;

    if (totalBytesLeftToRead > segmentBuffer->bufferSize - segmentBuffer->bufferDepth) {
        BDBG_ERR(("%s:%d: Segment Buffer is too small! Downloading %d bytes but buffer is only %d bytes",
                  __FUNCTION__, __LINE__,
                  (int)downloadContext->totalBytesExpected,
                  (int)(segmentBuffer->bufferSize - segmentBuffer->bufferDepth - downloadContext->totalBytesReadSoFar)));
        *serverClosed = true;
        rc = false;
    }

    PRINTMSG_SESSION(("%s: start downloading file: contentLength %lld, uri %s", __FUNCTION__, (long long)playback_ip->contentLength, playback_ip->openSettings.socketOpenSettings.url));
    /* start a timer to note the network b/w */
    B_Time_Get(&beginTime);
    while ( !*serverClosed) {
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode)) {
            PRINTMSG_SESSION(("%s: breaking file download loop due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
            break;
        }

        if (totalBytesLeftToRead <= 0) {
            /* we have read all the bytes that we need, so instead of
             * trying another read and waiting for server to close the
             * connection, consider this as server closed event and break
               out of the read loop */
            PRINTMSG_SESSION(("%s: breaking out of read loop as we have read %d upto the content length %lld", __FUNCTION__, (int)downloadContext->totalBytesReadSoFar, (long long)playback_ip->contentLength));
            *serverClosed = true;
            break;
        }

        bytesToRead = totalBytesLeftToRead; /* Remaining bytes to read. */

        if (bytesToRead > MPEG_DASH_READ_CHUNK_SIZE) {
             bytesToRead = MPEG_DASH_READ_CHUNK_SIZE;   /* Limit to our max chunk size. */
        }

        if ((bytesRead = playback_ip_read_socket( playback_ip,
                                                  playback_ip->securityHandle,
                                                  downloadContext->socketFd,
                                                  segmentBuffer->buffer + segmentBuffer->bufferDepth,
                                                  bytesToRead,
                                                  playback_ip->networkTimeout)) <= 0) {
            if (playback_ip->selectTimeout) {
                PRINTMSG_SESSION(("%s: socket error, retry read: size %d, errno :%d, state %d, select timeout %d, server closed %d",
                                  __FUNCTION__, (int)(downloadContext->totalBytesReadSoFar + bytesRead), errno, playback_ip->playback_state, playback_ip->selectTimeout, playback_ip->serverClosed));
                continue;
            }
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: Network Read Error, rc %d, playback ip state %d", __FUNCTION__, (int)bytesRead, playback_ip->playback_state));
#endif
            *serverClosed = true;
            break;
        }

        /* Account for the bytes just read.  */
        segmentBuffer->bufferDepth += bytesRead;
        downloadContext->totalBytesReadSoFar += bytesRead;
        totalBytesLeftToRead -= bytesRead;
    }

    if (*serverClosed == true) {
        /* close security context and socket */
        if (playback_ip->securityHandle) {
            playback_ip->netIo.close(playback_ip->securityHandle);
            playback_ip->securityHandle = NULL;
        }
        if (downloadContext->socketFd != -1) {
            close(downloadContext->socketFd);            /* Make sure that the socket is closed. */
            downloadContext->socketFd = -1;
        }
    }

    B_Time_Get(&endTime);
    totalDownloadTime = B_Time_Diff(&endTime, &downloadContext->beginTime);

    BDBG_NUL(("%s:%d: totalDownloadTime=%u", __FUNCTION__, __LINE__, totalDownloadTime));
    BDBG_NUL(("%s:%d: totalBytesRead=   %u", __FUNCTION__, __LINE__, downloadContext->totalBytesReadSoFar));

    if (totalDownloadTime == 0) totalDownloadTime = 1;

    downloadContext->elapsedTimeInMs = totalDownloadTime;

    PRINTMSG_SESSION(("%s: finished downloading (fd %d): errno %d, bytesRead: %d, state %d, select timeout %d, server closed %d",
                      __FUNCTION__, downloadContext->socketFd, errno, (int)downloadContext->totalBytesReadSoFar, playback_ip->playback_state, playback_ip->selectTimeout, playback_ip->serverClosed));

    /* downloaded the file into the buffer, now check if we need to decrypt this segment */

    BDBG_LEAVE(B_PlaybackIp_MpegDashMediaSegmentDownloadFinish);

    return rc;
}

/*****************************************************************************
 * Parse and split a URL string into its protocol, server, port and URL/URI
 * string.
 *****************************************************************************/
static bool
mpegDash_parse_absolute_url(B_PlaybackIpProtocol *protocol, char **server, unsigned *portPtr, char **uri, char *absoluteUri)
{
    char *tmp1, *tmp2, *tmp3;

    if ( (tmp1 = B_PlaybackIp_UtilsStristr(absoluteUri, "http://"))) {
        *protocol = B_PlaybackIpProtocol_eHttp;
        tmp1 += strlen("http://");
        *portPtr = 80;
    }
    else if ( (tmp1 = B_PlaybackIp_UtilsStristr(absoluteUri, "https://"))) {
        *protocol = B_PlaybackIpProtocol_eHttps;
        tmp1 += strlen("https://");
        *portPtr = 443;
    }
    else {
        BDBG_ERR(("%s: unsupported protocol in the given URL %s", __FUNCTION__, absoluteUri));
        return false;
    }
    /* http protocol is being used, parse it further */

    /* now take out the server string from the url */
    tmp2 = strstr(tmp1, "/");
    if (tmp2) {
        if ((*server = (char *)BKNI_Malloc(tmp2-tmp1+1)) == NULL) {
            BDBG_ERR(("%s: ERROR: failed to allocate %d bytes of memory at %d\n", __FUNCTION__, (int)(tmp2-tmp1), __LINE__));
            return false;
        }
        strncpy(*server, tmp1, tmp2-tmp1);
        (*server)[tmp2-tmp1] = '\0';

        /* Check to see if a port value was specified */
        tmp3 = strstr(*server, ":");
        if (tmp3) {
            tmp3[0] = '\0'; /* this null terminates the server name string */
            tmp3++;
            *portPtr = strtoul(tmp3, (char **) NULL, 10);
        }

        /* now get the uri */
        *uri = tmp2;
        PRINTMSG_URL(("%s: server %s, port %d, protocol %d, url %s", __FUNCTION__, *server, *portPtr, *protocol, *uri));
        return true;
    }
    else {
        BDBG_ERR(("%s: Incorrect URL: Failed to find the server part in %s", __FUNCTION__, absoluteUri));
        return false;
    }
}

/*****************************************************************************
 * Build an HTTP "GET" requst from the specified server, port, and URL.
 *
 * TODO: reuse the function from b_playback_ip_http.c
 *****************************************************************************/
static void
mpegDash_build_get_req(B_PlaybackIpHandle playback_ip, char *write_buf, int write_buf_size, char *server, int port, char *uri)
{
    snprintf(write_buf, write_buf_size,
            "GET %s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "User-Agent: %s\r\n"
            "Connection: Close\r\n"
            "EncEnabled: No\r\n"
            "transferMode.dlna.org: Streaming\r\n"
            "\r\n"
            ,uri, server, port,
            (playback_ip->openSettings.u.http.userAgent ? playback_ip->openSettings.u.http.userAgent : "BRCM HTTP Client/2.0")
            );
}


/*****************************************************************************
 * Begin a download (which can be finished by calling
 * B_PlaybackIp_MpegDashMediaSegmentDownloadFinish) from the
 * current network socket into the specified buffer and return
 * the number of bytes read.  Read until EOF, error condition,
 * or channel change (state change) occurs. In addition, also
 * measure the network bandwidth while downloading this data.
 * More specifically, this function is used to begin downloading
 * media segments and initialization segments.
 *****************************************************************************/
static bool
B_PlaybackIp_MpegDashMediaSegmentDownloadStart(
    B_PlaybackIpHandle playback_ip,
    MpegDashDownloadContext *downloadContext,
    MediaFileSegmentInfo    *mediaFileSegmentInfo,
    int                     *socketFd,
    MpegDashSegmentBuffer   *segmentBuffer
    )
{
    bool rc = false;
    char *server;
    unsigned port;
    char **uri;
    char *requestMessage, *responseMessage;
    char *http_hdr, *http_payload;
    http_url_type_t http_url_type;
    char *serverRedirect = NULL;
    char *uriRedirect = NULL;
    B_PlaybackIpProtocol protocol;
    B_PlaybackIpSessionOpenSettings openSettings;

    BDBG_ENTER(B_PlaybackIp_MpegDashMediaSegmentDownloadStart);

    if (!playback_ip || !mediaFileSegmentInfo || !downloadContext)
    {
        BDBG_LEAVE(B_PlaybackIp_MpegDashMediaSegmentDownloadStart);
        return false;
    }

    BKNI_Memset(downloadContext, 0, sizeof(*downloadContext));
    downloadContext->mediaFileSegmentInfo = mediaFileSegmentInfo;
    downloadContext->socketFd             = *socketFd;
    downloadContext->segmentBuffer        = segmentBuffer;

    B_Time_Get(&downloadContext->beginTime);

    if (!playback_ip->openSettings.socketOpenSettings.useProxy) {
        server = mediaFileSegmentInfo->server;
        port = mediaFileSegmentInfo->port;
    }
    else {
        /* when proxy is enabled, connection request has to be sent to the proxy server */
        server = playback_ip->openSettings.socketOpenSettings.ipAddr;
        port = playback_ip->openSettings.socketOpenSettings.port;
    }
    protocol = mediaFileSegmentInfo->protocol;
    uri = &mediaFileSegmentInfo->uri;
    PRINTMSG_SESSION(("%s: URI: http://%s:%d%s", __FUNCTION__, server, port, *uri));

    /* reset previous content length */
    playback_ip->contentLength = 0;

    /* prepare initial Get request */
    responseMessage = (char *) BKNI_Malloc(TMP_BUF_SIZE+1);
    requestMessage = (char *)BKNI_Malloc(TMP_BUF_SIZE+1);
    if (!responseMessage || !requestMessage) {
        BDBG_ERR(("%s: ERROR: failed to allocate memory\n", __FUNCTION__));
        goto error;
    }
    memset(&openSettings, 0, sizeof(openSettings));
    openSettings.security.securityProtocol = mediaFileSegmentInfo->securityProtocol;
    for (;;) {
        memset(requestMessage, 0, TMP_BUF_SIZE+1);
        memset(responseMessage, 0, TMP_BUF_SIZE+1);

        mpegDash_build_get_req(playback_ip, requestMessage, TMP_BUF_SIZE, server, port, *uri);

        /* setup the socket connection to the server & send GET request */
        if (B_PlaybackIp_UtilsTcpSocketConnect(&playback_ip->playback_state, server, port, false,  playback_ip->networkTimeout, socketFd) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to send Socket Connect Request to Server: %s:%d\n", __FUNCTION__, server, port));
            goto error;
        }
        downloadContext->socketFd = *socketFd;

        /* now setup security context prior to downloading the media segment */
        /* currently, supported security protocols are: HTTPS (SSL/TLS), AES128, and Clear (no encryption) */
        /* Note: security protocol can change from segment to segment, so this function is called prior to each segment download */
        openSettings.security.initialSecurityContext = playback_ip->openSettings.security.initialSecurityContext; /* points to either AES (dmaHandle) or SSL initial security context */
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
        openSettings.security.dmaHandle = playback_ip->openSettings.security.dmaHandle;
#endif
        switch (openSettings.security.securityProtocol) {
#ifdef B_HAS_HTTP_AES_SUPPORT
            case B_PlaybackIpSecurityProtocol_Aes128:
                /* setup the new key & iv */
                openSettings.security.enableDecryption = false;
                openSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_Aes128;
                memcpy(openSettings.security.settings.aes128.key, mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.key, sizeof(playback_ip->openSettings.security.settings.aes128.key));
                memcpy(openSettings.security.settings.aes128.iv, mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.iv, sizeof(playback_ip->openSettings.security.settings.aes128.iv));
                break;
#endif
            case B_PlaybackIpSecurityProtocol_Ssl:
                openSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_Ssl;
                openSettings.security.enableDecryption = true;
                break;
            default:
                /* Setup clear path */
                openSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_None;
        }
        if (B_PlaybackIp_SecuritySessionOpen(playback_ip, &openSettings, *socketFd, &playback_ip->securityHandle) < 0) {
            BDBG_ERR(("%s: ERROR: failed to setup the security session", __FUNCTION__));
            goto error;
        }

        /* and then send the HTTP Get request */
        if (playback_ip->netIo.write(playback_ip->securityHandle, &playback_ip->playback_state, *socketFd, requestMessage, strlen(requestMessage)) < 0) {
            BDBG_ERR(("%s: ERROR: failed to send HTTP Get request to Server: %s:%d\n", __FUNCTION__, server, port));
            goto error;
        }
        PRINTMSG_SESSION(("%s: Sent HTTP Get Request (socket %d) --->:\n %s", __FUNCTION__, *socketFd, requestMessage));

        playback_ip->chunkEncoding = false;
        playback_ip->serverClosed = false;
        playback_ip->selectTimeout = false;

        /* now read and process the HTTP Response headers */
        if (http_read_response(playback_ip, playback_ip->securityHandle, *socketFd, &responseMessage, TMP_BUF_SIZE, &http_hdr, &http_payload, &playback_ip->initial_data_len) < 0) {
            BDBG_ERR(("%s: ERROR: failed to receive valid HTTP response\n", __FUNCTION__));
            goto error;
        }

        PRINTMSG_SESSION(("%s: http_hdr Offset: %d", __FUNCTION__, (int)(http_hdr - responseMessage)));
        PRINTMSG_SESSION(("%s: http_payload Offset: %d", __FUNCTION__, (int)(http_payload - responseMessage)));
        PRINTMSG_SESSION(("%s: initial_data_len: %d", __FUNCTION__, playback_ip->initial_data_len));


        http_url_type = http_get_url_type(http_hdr, *uri);
        PRINTMSG_SESSION(("%s: http url type %d", __FUNCTION__, http_url_type));
        if (http_url_type == HTTP_URL_IS_REDIRECT) {
            /* parse HTTP redirect and extract the new URI & server:port info */
            if ((serverRedirect = (char *)BKNI_Malloc(512)) == NULL) {
                BDBG_ERR(("%s: failed to allocate memory for redirectServer", __FUNCTION__));
                goto error;
            }
            if (http_parse_redirect(serverRedirect, &port, &protocol, &uriRedirect, &playback_ip->cookieFoundViaHttpRedirect, http_hdr) != 0) {
                BDBG_ERR(("%s: Incorrect HTTP Redirect response or parsing error", __FUNCTION__));
                goto error;
            }
            /* previous function gets the new URL & server information and we send another GET request to this server */
            close(*socketFd);
            if (playback_ip->securityHandle)
                playback_ip->netIo.close(playback_ip->securityHandle);
            playback_ip->securityHandle = NULL;
            server = serverRedirect;
            uri = &uriRedirect;
            if (protocol == B_PlaybackIpProtocol_eHttps)
                openSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_Ssl;
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: HTTP redirect case, caching the redirected URL: http://%s:%d%s", __FUNCTION__, server, port, *uri));
#endif
            continue;
        }
        else {
            /* actual content URL, get the content attributes from HTTP response header */
            PRINTMSG_SESSION(("%s: GOT ACTUAL CONTENT: sock fd %d", __FUNCTION__, *socketFd));
            break;
        }
    }

    /* store the content type of the payload, this assists in providing a valuble hint to media probe and thus cuts down the probe time */
    playback_ip->http_content_type = http_get_payload_content_type(responseMessage);

#ifdef B_HAS_HTTP_AES_SUPPORT
    if (openSettings.security.securityProtocol == B_PlaybackIpSecurityProtocol_Aes128) {
        /* enable AES encryption as HTTP header processing (which is in clear) is done now */
        if (B_PlaybackIp_AesDecryptionEnable(playback_ip->securityHandle, http_payload, playback_ip->initial_data_len) < 0) {
            BDBG_ERR(("%s: ERROR: failed to enable the security decryption", __FUNCTION__));
            goto error;
        }
        PRINTMSG_SESSION(("%s: security context is enabled for media segment %p, seq # %d, sec protocol %d, initial encrypted bytes %d", __FUNCTION__, (void *)mediaFileSegmentInfo, mediaFileSegmentInfo->mediaSequence, openSettings.security.securityProtocol, playback_ip->initial_data_len));
        playback_ip->initial_data_len = 0; /* initial payload is now given to security layer for later decryption during the read call */
    }
#endif /* B_HAS_HTTP_AES_SUPPORT */

    downloadContext->totalBytesExpected = segmentBuffer->bufferSize - segmentBuffer->bufferDepth;
    if (playback_ip->contentLength > 0) {
        downloadContext->totalBytesExpected = playback_ip->contentLength;
    }
    /* At this point, it's possible that totalBytesExpected can
     * exceed the available space in the buffer.  If so, that will
     * be handled in
     * B_PlaybackIp_MpegDashMediaSegmentDownloadFinish() */

    if (playback_ip->initial_data_len) {
        ssize_t bytesToCopy = playback_ip->initial_data_len;

        if (bytesToCopy > downloadContext->totalBytesExpected) {
            bytesToCopy = downloadContext->totalBytesExpected;
        }
        if (bytesToCopy > segmentBuffer->bufferSize - segmentBuffer->bufferDepth) {
            bytesToCopy = segmentBuffer->bufferSize - segmentBuffer->bufferDepth;
        }
        memcpy(segmentBuffer->buffer + segmentBuffer->bufferDepth, http_payload, bytesToCopy);
        segmentBuffer->bufferDepth += bytesToCopy;
        downloadContext->totalBytesReadSoFar += bytesToCopy;
        PRINTMSG_SESSION(("%s: completed, %d bytes of initial data read", __FUNCTION__, playback_ip->initial_data_len));
    }
    rc = true;

error:
    if (serverRedirect)
        BKNI_Free(serverRedirect);
    if (uriRedirect)
        BKNI_Free(uriRedirect); /* allocated via strdup of redirected URI in http_parse_redirect */
    if (responseMessage)
        BKNI_Free(responseMessage);
    if (requestMessage)
        BKNI_Free(requestMessage);
    BDBG_LEAVE(B_PlaybackIp_MpegDashMediaSegmentDownloadStart);
    return rc;
}


/*****************************************************************************
 * Define some macros that are used to generate code for the object
 * constructors and destructors.  Using these macros will eliminate most
 * of the common code that would otherwise exist for each type (class) of
 * object.
 *****************************************************************************/

/*****************************************************************************
 * Create, allocate, and zero-fill storage for the object.  Print debug
 * message as needed.
 *****************************************************************************/
#define MPEGDASH_CREATE_ALLOCATE( destPtr, xmlElem, structType)                                                                             \
    do {                                                                                                                                    \
        PRINT_XML_DEBUG(("%s: Creating %s Object (%u bytes) for xmlElem: %p", __FUNCTION__, #structType, (int)sizeof (structType), (void *)xmlElem), xmlElem ); \
        PRINTMSG_OBJECTS(("%s: Creating %s Object (%u bytes) for xmlElem: %p", __FUNCTION__, #structType, (int)sizeof (structType), (void *)xmlElem)); \
                                                                                                                                            \
        /* Allocate a new Info structure */                                                                                                 \
        if ((destPtr = (structType *)BKNI_Malloc(sizeof (structType))) == NULL) {                                                           \
            BDBG_ERR(("%s: Failed to allocate %d bytes of memory for Info structure", __FUNCTION__, (int)sizeof (structType) )); \
            return NULL;                                                                                                                    \
        }                                                                                                                                   \
        memset(destPtr, 0, sizeof (structType) );                                                                                           \
                                                                                                                                            \
        destPtr->myXmlElem         = xmlElem;                                                                                               \
    } while(0)


/*****************************************************************************
 * Using a specified "XmlElement", create a list of the specified child
 * subelements by calling the child's constructor.  The new child, in turn,
 * will also create it's children subelements (and their subelements)
 * before returning.   Each newly created subelement is added to the
 * appropriate linked list belonging to the parent element.
 * The subelement list is used for sublements which can have more than one
 * instance.
 *****************************************************************************/
#define MPEGDASH_CREATE_SUBELEM_LIST(listHead,xmlElemParent,xmlTagChild,structTypeChild,constructorChild,linkChild,errorLabel)     \
    do {                                                                                                                           \
        B_PlaybackIp_XmlElement xmlElemChild = NULL; /* start with first child element */                                          \
                                                                                                                                   \
        PRINTMSG_OBJECTS(("%s: Creating Subelement list for %s Objects", __FUNCTION__, #structTypeChild));                         \
                                                                                                                                   \
        BLST_Q_INIT(&listHead);                                                                                                    \
        for (;;)                                                                                                                   \
        {                                                                                                                          \
            structTypeChild * pNewChild;                                                                                           \
                                                                                                                                   \
            /*  Now get the first/next child under the parent (if there are any). */                                               \
            xmlElemChild = B_PlaybackIp_XmlElem_FindNextChildSameTag(xmlElemParent, xmlElemChild, xmlTagChild);                    \
            if ( !xmlElemChild) break;    /* No more children... all done. */                                                      \
                                                                                                                                   \
            PRINT_XML_DEBUG(("%s: Adding %s: %p", __FUNCTION__, #structTypeChild, (void *)xmlElemChild), (void *)xmlElemChild );                   \
                                                                                                                                   \
            pNewChild = constructorChild(playback_ip, xmlElemChild);                                                               \
            if (!pNewChild)  goto errorLabel;                                                                                      \
                                                                                                                                   \
            BLST_Q_INSERT_TAIL(&listHead, pNewChild, linkChild);                                                                   \
        }                                                                                                                          \
    } while(0)

/*****************************************************************************
 * Using a specified "XmlElement", create a single instance of the specified
 * child subelement by calling the child's constructor.  The new child, in turn,
 * will also create it's children subelements (and their subelements)
 * before returning.   A pointer in the parent element is set to point to the
 * newly created child element.
 *****************************************************************************/
#define MPEGDASH_CREATE_SUBELEMT( destPtr, xmlElemParent, xmlTagChild, structTypeChild, constructorChild, errorLabel )    \
    do {                                                                                                                  \
        B_PlaybackIp_XmlElement xmlElemChild = NULL; /* start with first child element */                                 \
                                                                                                                          \
        PRINTMSG_OBJECTS(("%s: Creating Subelement for %s Object", __FUNCTION__, #structTypeChild));                      \
                                                                                                                          \
        xmlElemChild = B_PlaybackIp_XmlElem_FindChild( xmlElemParent, xmlTagChild);                                       \
        if ( xmlElemChild)                                                                                                \
        {                                                                                                                 \
            PRINT_XML_DEBUG(("%s: Adding %s: %p", __FUNCTION__, #structTypeChild, (void *)xmlElemChild), (void *)xmlElemChild );          \
            destPtr = constructorChild(playback_ip, xmlElemChild);                                                        \
            if (!destPtr) goto errorLabel;                                                                                \
        }                                                                                                                 \
    } while(0)

/*****************************************************************************
 * For each child sublement of the specified type/class update its
 * parentObj pointer to point back to its parent. This can only be used
 * for child subelements that have a well-defined parent type/class...
 * probably just for Representations, AdaptationSets, and Periods.
 *****************************************************************************/
#define MPEGDASH_SET_SUBELEM_LIST_PARENT(listHead, structTypeChild, parent, linkChild)                        \
    do {                                                                                                      \
        structTypeChild   * childInfo;                                                                        \
        /* Set each subelement to point to its parent */                                                      \
                                                                                                              \
        for(childInfo = BLST_Q_FIRST(&listHead) ; childInfo ; childInfo = BLST_Q_NEXT(childInfo, linkChild) ) \
        {                                                                                                     \
            childInfo->parentObj = parent;                                                                    \
            PRINTMSG_OBJECTS(("%s: Set parentObj for %s element", __FUNCTION__, #structTypeChild ));          \
        }                                                                                                     \
    } while(0)


/*****************************************************************************
 * Destroy the specified object and deallocate its storage.  Print an
 * appropriate debug message as needed.
 *****************************************************************************/
#define MPEGDASH_DESTROY_DEALLOCATE( destPtr, structType)                                                                           \
    do {                                                                                                                            \
        structType * myPtr = destPtr;                                                                                               \
                                                                                                                                    \
        if (myPtr) {                                                                                                                \
            /* Deallocate the object's structure. */                                                                                \
            PRINTMSG_OBJECTS(("%s: Destroying %s subelement %p, freeing %u bytes", __FUNCTION__, #structType, (void *)myPtr, (int)sizeof (structType))); \
            BKNI_Free( myPtr);                                                                                                      \
            myPtr = NULL;                                                                                                           \
        }                                                                                                                           \
    } while(0)


/*****************************************************************************
 * For each child element of the specified type, unlink it from its parent's
 * linked list, then destroy the child.
 *****************************************************************************/
#define MPEGDASH_DESTROY_SUBELEM_LIST(listHead, structTypeChild, destructorChild, linkChild)                    \
    do {                                                                                                        \
        structTypeChild   * childInfo;                                                                          \
                                                                                                                \
        /* Destroy each subelement */                                                                           \
        for(childInfo = BLST_Q_FIRST(&listHead) ; childInfo ; childInfo = BLST_Q_FIRST(&listHead))              \
        {                                                                                                       \
            PRINTMSG_OBJECTS(("%s: Destroying %s subelement %p", __FUNCTION__, #structTypeChild, (void *)childInfo ));  \
            BLST_Q_REMOVE(&listHead, childInfo, linkChild);                                                     \
                                                                                                                \
            destructorChild(childInfo);                                                                         \
        }                                                                                                       \
    } while(0)


/*****************************************************************************
 * For a single child element of the specified type, set it's parent's pointer
 * to NULL, then destroy the child.
 *****************************************************************************/
#define MPEGDASH_DESTROY_SUBELEM( destPtr, structTypeChild, destructorChild )                              \
    do {                                                                                                   \
        structTypeChild   * myPtr = destPtr;                                                               \
        /* Destroy the subelement if it exists */                                                          \
        if (myPtr) {                                                                                       \
            PRINTMSG_OBJECTS(("%s: Destroying %s subelement %p", __FUNCTION__, #structTypeChild, (void *)myPtr )); \
            destructorChild(myPtr);                                                                        \
            myPtr = NULL;                                                                                  \
        }                                                                                                  \
        else                                                                                               \
        {                                                                                                  \
            PRINTMSG_OBJECTS(("%s: No %s subelements to destroy", __FUNCTION__, #structTypeChild ));       \
        }                                                                                                  \
    } while(0)


/********************* Create/destroy "AnyTextType" objects *******************/
static void B_PlaybackIp_MpegDashDestroyAnyText(MpegDashAnyTextType     *anyText)
{
    /* Destroy subelements...*/
    /* No subelements for AnyTextType  */

    /* Deallocate the AnyTextType structure. */
    MPEGDASH_DESTROY_DEALLOCATE( anyText, MpegDashAnyTextType);
}

static MpegDashAnyTextType *
B_PlaybackIp_MpegDashCreateAnyText(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement   xmlElem)
{
    MpegDashAnyTextType     * anyText;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( anyText, xmlElem, MpegDashAnyTextType);

    anyText->text          = B_PlaybackIp_XmlElem_ChildData(xmlElem);

    /* Create subelements...*/
    /* No subelements for AnyTextType  */

    return anyText;
}


/********************* Create/destroy "BaseUrlInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroyBaseUrl(MpegDashBaseUrlInfo     *baseUrlInfo)
{
    /* Destroy subelements...*/
    /* No subelements for BaseUrlInfo  */

    /* Deallocate the BaseUrlInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( baseUrlInfo, MpegDashBaseUrlInfo);
}

static MpegDashBaseUrlInfo *
B_PlaybackIp_MpegDashCreateBaseUrl(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement   xmlElem)
{
    MpegDashBaseUrlInfo     * baseUrlInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( baseUrlInfo, xmlElem, MpegDashBaseUrlInfo);

    baseUrlInfo->childData          = B_PlaybackIp_XmlElem_ChildData(xmlElem);
    baseUrlInfo->serviceLocation    = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_SERVICELOCATION);
    baseUrlInfo->byteRange          = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_BYTERANGE);

    /* Create subelements...*/
    /* No subelements for BaseUrlInfo  */

    return baseUrlInfo;
}

/********************* Create/destroy "UrlInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroyUrlInfo(MpegDashUrlInfo     *urlInfo)
{
    /* Destroy subelements...*/
    /* No subelements for UrlInfo  */

    /* Deallocate the UrlInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( urlInfo, MpegDashUrlInfo);
}

static MpegDashUrlInfo *
B_PlaybackIp_MpegDashCreateUrl(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement xmlElem)
{
    MpegDashUrlInfo     * urlInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( urlInfo, xmlElem, MpegDashUrlInfo);

    urlInfo->sourceURL  = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_SOURCEURL   );
    urlInfo->range      = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_RANGE       );

    /* Create subelements...*/
    /* No subelements for UrlInfo  */

    return urlInfo;
}


/********************* Create/destroy "SegmentUrlInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroySegmentUrl(MpegDashSegmentUrlInfo     *segmentUrlInfo)
{
    /* Destroy subelements...*/
    /* No subelements for SegmentUrlInfo  */

    /* Deallocate the SegmentUrlInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( segmentUrlInfo, MpegDashSegmentUrlInfo);
}

static MpegDashSegmentUrlInfo *
B_PlaybackIp_MpegDashCreateSegmentUrl(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement xmlElem)
{
    MpegDashSegmentUrlInfo     * segmentUrlInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( segmentUrlInfo, xmlElem, MpegDashSegmentUrlInfo);

    segmentUrlInfo->media        = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_MEDIA            );
    segmentUrlInfo->mediaRange   = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_MEDIARANGE       );
    segmentUrlInfo->index        = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_INDEX            );
    segmentUrlInfo->indexRange   = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_INDEXRANGE       );

    return segmentUrlInfo;
}


/********************* Create/destroy "RepresentationBaseInfo" base objects *******************/
static void
B_PlaybackIp_MpegDashDestroyRepresentationBase(MpegDashRepresentationBaseInfo *representationBase)
{
    /* Destroy subelements...*/
    /* Destroy FramePacking Subelements */
    /* Destroy AudioChannelConfiguration Subelements */
    /* Destroy ContentProtection Subelements */

    /* Deallocate the RepresentationBaseInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( representationBase, MpegDashRepresentationBaseInfo);
}

static MpegDashRepresentationBaseInfo *
B_PlaybackIp_MpegDashCreateRepresentationBase(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement   xmlElem)
{
    MpegDashRepresentationBaseInfo     * representationBaseInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( representationBaseInfo, xmlElem, MpegDashRepresentationBaseInfo);

    representationBaseInfo->profiles          = B_PlaybackIp_XmlElem_FindAttrValue(         xmlElem , XML_ATT_PROFILES          );
    representationBaseInfo->width             = B_PlaybackIp_XmlElem_FindAttrValueUnsigned( xmlElem , XML_ATT_WIDTH        , 0  );
    representationBaseInfo->height            = B_PlaybackIp_XmlElem_FindAttrValueUnsigned( xmlElem , XML_ATT_HEIGHT       , 0  );
    representationBaseInfo->sar               = B_PlaybackIp_XmlElem_FindAttrValue(         xmlElem , XML_ATT_SAR               );
    representationBaseInfo->frameRate         = B_PlaybackIp_XmlElem_FindAttrValue(         xmlElem , XML_ATT_FRAMERATE         );
    representationBaseInfo->audioSamplingRate = B_PlaybackIp_XmlElem_FindAttrValue(         xmlElem , XML_ATT_AUDIOSAMPLINGRATE );
    representationBaseInfo->mimeType          = B_PlaybackIp_XmlElem_FindAttrValue(         xmlElem , XML_ATT_MIMETYPE          );
    representationBaseInfo->segmentProfiles   = B_PlaybackIp_XmlElem_FindAttrValue(         xmlElem , XML_ATT_SEGMENTPROFILES   );
    representationBaseInfo->codecs            = B_PlaybackIp_XmlElem_FindAttrValue(         xmlElem , XML_ATT_CODECS            );
    representationBaseInfo->maximumSAPPeriod  = B_PlaybackIp_XmlElem_FindAttrValue(         xmlElem , XML_ATT_MAXIMUMSAPPERIOD  );
    representationBaseInfo->startWithSAP      = B_PlaybackIp_XmlElem_FindAttrValueUnsigned( xmlElem , XML_ATT_STARTWITHSAP , 0  );
    representationBaseInfo->maxPlayoutRate    = B_PlaybackIp_XmlElem_FindAttrValue(         xmlElem , XML_ATT_MAXPLAYOUTRATE    );
    representationBaseInfo->codingDependency  = B_PlaybackIp_XmlElem_FindAttrValueBoolean(  xmlElem , XML_ATT_CODINGDEPENDENCY, false  );
    representationBaseInfo->scanType          = B_PlaybackIp_XmlElem_FindAttrValue(         xmlElem , XML_ATT_SCANTYPE          );

    /* Create subelements...*/
    /* Create FramePacking Subelements */
    /* Create AudioChannelConfiguration Subelements */
    /* Create ContentProtection Subelements */

    return representationBaseInfo;
}


/********************* Create/destroy "SegmentBaseInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroySegmentBase(MpegDashSegmentBaseInfo *segmentBaseInfo)
{

    /* Destroy subelements...*/
    /* Destroy the RepresentationIndex subelement */
    MPEGDASH_DESTROY_SUBELEM( segmentBaseInfo->representationIndex, MpegDashUrlInfo, B_PlaybackIp_MpegDashDestroyUrlInfo);

    /* Destroy Initialization subelement */
    MPEGDASH_DESTROY_SUBELEM( segmentBaseInfo->initialization, MpegDashUrlInfo, B_PlaybackIp_MpegDashDestroyUrlInfo);

    /* Deallocate the SegmentBaseInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( segmentBaseInfo, MpegDashSegmentBaseInfo);
}

static MpegDashSegmentBaseInfo *
B_PlaybackIp_MpegDashCreateSegmentBase(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement xmlElem)
{
    MpegDashSegmentBaseInfo     * segmentBaseInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( segmentBaseInfo, xmlElem, MpegDashSegmentBaseInfo);

    segmentBaseInfo->timescale                = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_TIMESCALE              );
    segmentBaseInfo->presentationTimeOffset   = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_PRESENTATIONTIMEOFFSET );
    segmentBaseInfo->indexRange               = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_INDEXRANGE             );
    segmentBaseInfo->indexRangeExact          = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_INDEXRANGEEXACT        );

    /* Create subelements...*/
    /* Create Initialization Subelement */
    MPEGDASH_CREATE_SUBELEMT( segmentBaseInfo->initialization, xmlElem, XML_TAG_INITIALIZATIONSEGMENT, MpegDashUrlInfo, B_PlaybackIp_MpegDashCreateUrl, SegmentBaseCreateError );

    /* Create RepresentationIndex Subelement */
    MPEGDASH_CREATE_SUBELEMT( segmentBaseInfo->representationIndex, xmlElem, XML_TAG_REPRESENTATIONINDEX, MpegDashUrlInfo, B_PlaybackIp_MpegDashCreateUrl, SegmentBaseCreateError );

    return segmentBaseInfo;

SegmentBaseCreateError:
    B_PlaybackIp_MpegDashDestroySegmentBase(segmentBaseInfo);
    return NULL;
}


/********************* Create/destroy "SegmentTimelineElemInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroySegmentTimelineElem(MpegDashSegmentTimelineElemInfo *segmentTimelineElemInfo)
{

    /* Destroy subelements...*/
    /*   No subelements.  */

    /* Deallocate the SegmentTimelineElemInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( segmentTimelineElemInfo, MpegDashSegmentTimelineElemInfo);
}

static MpegDashSegmentTimelineElemInfo *
B_PlaybackIp_MpegDashCreateSegmentTimelineElem(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement xmlElem)
{
    MpegDashSegmentTimelineElemInfo     * segmentTimelineElemInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( segmentTimelineElemInfo, xmlElem, MpegDashSegmentTimelineElemInfo);

    segmentTimelineElemInfo->t                = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_SEGMENTTIMELINE_T);
    segmentTimelineElemInfo->d                = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_SEGMENTTIMELINE_D);
    segmentTimelineElemInfo->r                = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_SEGMENTTIMELINE_R);

    /* Convert the DateTime and Duration attributes to their "cooked" internal
     * formats. */
    if (segmentTimelineElemInfo->t) {
        segmentTimelineElemInfo->cooked.t =  strtoul(segmentTimelineElemInfo->t, NULL, 10);
    }
    if (segmentTimelineElemInfo->d) {
        segmentTimelineElemInfo->cooked.d =  strtoul(segmentTimelineElemInfo->d, NULL, 10);
    }
    if (segmentTimelineElemInfo->r) {
        segmentTimelineElemInfo->cooked.r =  strtoul(segmentTimelineElemInfo->r, NULL, 10);
    }


    /* Create subelements...*/
    /*   No subelements. */

    return segmentTimelineElemInfo;
}


/********************* Create/destroy "MpegDashSegmentTimelineInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroySegmentTimeline(MpegDashSegmentTimelineInfo *segmentTimelineInfo)
{
    /* Destroy subelements...*/
    /* Destroy SegmentTimeline "S" Subelements */
    MPEGDASH_DESTROY_SUBELEM_LIST(segmentTimelineInfo->mpegDashSegmentTimelineElemInfoHead, MpegDashSegmentTimelineElemInfo, B_PlaybackIp_MpegDashDestroySegmentTimelineElem, nextSegmentTimelineElemInfo);

    /* Deallocate the SegmentTimelineInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( segmentTimelineInfo, MpegDashSegmentTimelineInfo);
}

static MpegDashSegmentTimelineInfo *
B_PlaybackIp_MpegDashCreateSegmentTimeline(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement xmlElem)
{
    MpegDashSegmentTimelineInfo *segmentTimelineInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( segmentTimelineInfo, xmlElem, MpegDashSegmentTimelineInfo);

    /* Create subelements...*/
    /* Create SegmentTimeline "S" Subelements */
    MPEGDASH_CREATE_SUBELEM_LIST( segmentTimelineInfo->mpegDashSegmentTimelineElemInfoHead, xmlElem, XML_TAG_SEGMENTTIMELINEELEM, MpegDashSegmentTimelineElemInfo, B_PlaybackIp_MpegDashCreateSegmentTimelineElem, nextSegmentTimelineElemInfo, SegmentTimelineCreateError );

    return segmentTimelineInfo;

SegmentTimelineCreateError:
    B_PlaybackIp_MpegDashDestroySegmentTimeline(segmentTimelineInfo);
    return NULL;
}


/********************* Create/destroy "MultipleSegmentBaseInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroyMultipleSegmentBase(MpegDashMultipleSegmentBaseInfo *multipleSegmentBaseInfo)
{
    /* Destroy subelements...*/
    /* Destroy BitstreamSwitching Subelement */
    MPEGDASH_DESTROY_SUBELEM( multipleSegmentBaseInfo->bitstreamSwitching, MpegDashUrlInfo,             B_PlaybackIp_MpegDashDestroyUrlInfo);
    MPEGDASH_DESTROY_SUBELEM( multipleSegmentBaseInfo->segmentTimeline,    MpegDashSegmentTimelineInfo, B_PlaybackIp_MpegDashDestroySegmentTimeline);

    /* Destroy the SegmentBase base class objects */
    if (multipleSegmentBaseInfo->baseTypeSegmentBase) {
        B_PlaybackIp_MpegDashDestroySegmentBase(multipleSegmentBaseInfo->baseTypeSegmentBase);
    }

    /* Deallocate the MultipleSegmentBaseInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( multipleSegmentBaseInfo, MpegDashMultipleSegmentBaseInfo);
}

static MpegDashMultipleSegmentBaseInfo *
B_PlaybackIp_MpegDashCreateMultipleSegmentBase(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement xmlElem)
{
    MpegDashMultipleSegmentBaseInfo     * multipleSegmentBaseInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( multipleSegmentBaseInfo, xmlElem, MpegDashMultipleSegmentBaseInfo);

    /* Create base class objects */
    multipleSegmentBaseInfo->baseTypeSegmentBase = B_PlaybackIp_MpegDashCreateSegmentBase(playback_ip, xmlElem);
    if (!multipleSegmentBaseInfo->baseTypeSegmentBase) goto MultipleSegmentBaseCreateError;

    multipleSegmentBaseInfo->duration       = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_DURATION    );
    multipleSegmentBaseInfo->startNumber    = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_STARTNUMBER );

    /* Create subelements...*/
    /* Create BitstreamSwitching Subelement */
    MPEGDASH_CREATE_SUBELEMT( multipleSegmentBaseInfo->bitstreamSwitching, xmlElem, XML_TAG_BITSTREAMSWITCHING, MpegDashUrlInfo,             B_PlaybackIp_MpegDashCreateUrl,             MultipleSegmentBaseCreateError );
    MPEGDASH_CREATE_SUBELEMT( multipleSegmentBaseInfo->segmentTimeline,    xmlElem, XML_TAG_SEGMENTTIMELINE,    MpegDashSegmentTimelineInfo, B_PlaybackIp_MpegDashCreateSegmentTimeline, MultipleSegmentBaseCreateError );

    return multipleSegmentBaseInfo;

MultipleSegmentBaseCreateError:
    B_PlaybackIp_MpegDashDestroyMultipleSegmentBase(multipleSegmentBaseInfo);
    return NULL;
}


/********************* Create/destroy "SegmentListInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroySegmentList(MpegDashSegmentListInfo *segmentListInfo)
{
    /* Destroy subelements...*/
    /* Destroy SegmentUrl Subelements */
    MPEGDASH_DESTROY_SUBELEM_LIST(segmentListInfo->mpegDashSegmentUrlInfoHead, MpegDashSegmentUrlInfo, B_PlaybackIp_MpegDashDestroySegmentUrl, nextSegmentUrl);

    /* Destroy the MultipleSegmentBase base class objects */
    if (segmentListInfo->baseTypeMultipleSegmentBase) {
        B_PlaybackIp_MpegDashDestroyMultipleSegmentBase(segmentListInfo->baseTypeMultipleSegmentBase);
    }

    /* Deallocate the SegmentListInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( segmentListInfo, MpegDashSegmentListInfo);
}

static MpegDashSegmentListInfo *
B_PlaybackIp_MpegDashCreateSegmentList(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement xmlElem)
{
    MpegDashSegmentListInfo *segmentListInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( segmentListInfo, xmlElem, MpegDashSegmentListInfo);

    /* Create base class objects */
    segmentListInfo->baseTypeMultipleSegmentBase = B_PlaybackIp_MpegDashCreateMultipleSegmentBase(playback_ip, xmlElem);
    if (!segmentListInfo->baseTypeMultipleSegmentBase) goto SegmentListCreateError;

    segmentListInfo->href         = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_HREF     );
    segmentListInfo->actuate      = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_ACTUATE  );

    /* Create subelements...*/
    /* Create SegmentURL Subelements */
    MPEGDASH_CREATE_SUBELEM_LIST( segmentListInfo->mpegDashSegmentUrlInfoHead, xmlElem, XML_TAG_SEGMENTURL, MpegDashSegmentUrlInfo, B_PlaybackIp_MpegDashCreateSegmentUrl, nextSegmentUrl, SegmentListCreateError );

    return segmentListInfo;

SegmentListCreateError:
    B_PlaybackIp_MpegDashDestroySegmentList(segmentListInfo);
    return NULL;
}


/********************* Create/destroy "SegmentTemplateInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroySegmentTemplate(MpegDashSegmentTemplateInfo *segmentTemplateInfo)
{
    /* Destroy subelements...*/
    /* No subelements for SegmentTemplateInfo  */

    /* Destroy the MultipleSegmentBase base class objects */
    if (segmentTemplateInfo->baseTypeMultipleSegmentBase) {
        B_PlaybackIp_MpegDashDestroyMultipleSegmentBase(segmentTemplateInfo->baseTypeMultipleSegmentBase);
    }

    /* Deallocate the SegmentTemplateInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( segmentTemplateInfo, MpegDashSegmentTemplateInfo);
}

static MpegDashSegmentTemplateInfo *
B_PlaybackIp_MpegDashCreateSegmentTemplate(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement xmlElem)
{
    MpegDashSegmentTemplateInfo *segmentTemplateInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( segmentTemplateInfo, xmlElem, MpegDashSegmentTemplateInfo);

    /* Create base class objects */
    segmentTemplateInfo->baseTypeMultipleSegmentBase = B_PlaybackIp_MpegDashCreateMultipleSegmentBase(playback_ip, xmlElem);
    if (!segmentTemplateInfo->baseTypeMultipleSegmentBase) goto SegmentTemplateCreateError;

    segmentTemplateInfo->media                = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_MEDIA               );
    segmentTemplateInfo->index                = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_INDEX               );
    segmentTemplateInfo->initialization       = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_INITIALIZATION      );
    segmentTemplateInfo->bitstreamSwitching   = B_PlaybackIp_XmlElem_FindAttrValue( xmlElem , XML_ATT_BITSTREAMSWITCHING  );

    /* Create subelements...*/
    /* No subelements for SegmentTemplateInfo  */
    return segmentTemplateInfo;

SegmentTemplateCreateError:
    B_PlaybackIp_MpegDashDestroySegmentTemplate(segmentTemplateInfo);
    return NULL;
}


/********************* Create/destroy "RepresentationInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroyRepresentation(MpegDashRepresentationInfo *representationInfo)
{
    /* Free up some BKNI_Malloc'd buffers if they've been allocated. */
    if (representationInfo->codecSpecificAddrForAudio) BKNI_Free(representationInfo->codecSpecificAddrForAudio);
    if (representationInfo->codecSpecificAddrForVideo) BKNI_Free(representationInfo->codecSpecificAddrForVideo);

    /* Destroy subelements...*/
    /* Destroy SegmentList (if there is one) */
    MPEGDASH_DESTROY_SUBELEM( representationInfo->segmentList, MpegDashSegmentListInfo, B_PlaybackIp_MpegDashDestroySegmentList);

    /* Destroy SegmentBase (if there is one) */
    MPEGDASH_DESTROY_SUBELEM( representationInfo->segmentBase, MpegDashSegmentBaseInfo, B_PlaybackIp_MpegDashDestroySegmentBase);

    /* Destroy SegmentTemplate (if there is one) */
    MPEGDASH_DESTROY_SUBELEM( representationInfo->segmentTemplate, MpegDashSegmentTemplateInfo, B_PlaybackIp_MpegDashDestroySegmentTemplate);

    /* Destroy BaseUrls */
    MPEGDASH_DESTROY_SUBELEM_LIST(representationInfo->mpegDashBaseUrlInfoHead, MpegDashBaseUrlInfo, B_PlaybackIp_MpegDashDestroyBaseUrl, nextBaseUrlInfo);

    /* Destroy the RepresentationBase base class objects */
    if (representationInfo->baseTypeRepresentationBase) {
        B_PlaybackIp_MpegDashDestroyRepresentationBase(representationInfo->baseTypeRepresentationBase);
    }

    /* Deallocate the RepresentationInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( representationInfo, MpegDashRepresentationInfo);
}

static MpegDashRepresentationInfo *
B_PlaybackIp_MpegDashCreateRepresentation(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement   xmlElem)
{
    MpegDashRepresentationInfo             * representationInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( representationInfo, xmlElem, MpegDashRepresentationInfo);

    /* First, get the fields for the RepresentationBase structure. */
    representationInfo->baseTypeRepresentationBase  = B_PlaybackIp_MpegDashCreateRepresentationBase(playback_ip, xmlElem);

    representationInfo->id                       = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_ID                     );
    representationInfo->bandwidth                = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_BANDWIDTH              );
    representationInfo->bandwidthNumeric         = B_PlaybackIp_XmlElem_FindAttrValueUnsigned(xmlElem, XML_ATT_BANDWIDTH          , 0 );
    representationInfo->qualityRanking           = B_PlaybackIp_XmlElem_FindAttrValueUnsigned(xmlElem, XML_ATT_QUALITYRANKING     , 0 );
    representationInfo->dependencyId             = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_DEPENDENCYID           );
    representationInfo->mediaStreamStructureId   = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_MEDIASTREAMSTRUCTUREID );

    /* Create subelements...*/

    /* Create BaseURL Subelements */
    MPEGDASH_CREATE_SUBELEM_LIST( representationInfo->mpegDashBaseUrlInfoHead, xmlElem, XML_TAG_BASEURL, MpegDashBaseUrlInfo, B_PlaybackIp_MpegDashCreateBaseUrl, nextBaseUrlInfo, RepresentationCreateError );

    /* GARYWASHERE: Also need to add the following subelements to the Representation. */
    /*  BLST_Q_HEAD(mpegDashSubRepresentationInfoHead,  MpegDashSubRepresentationInfo)  mpegDashSubRepresentationInfoHead; */


    /* Create the SegmentTemplate subelement (if there is one) */
    MPEGDASH_CREATE_SUBELEMT( representationInfo->segmentTemplate, xmlElem, XML_TAG_SEGMENTTEMPLATE, MpegDashSegmentTemplateInfo, B_PlaybackIp_MpegDashCreateSegmentTemplate, RepresentationCreateError );

    /* Create the SegmentBase subelement (if there is one) */
    MPEGDASH_CREATE_SUBELEMT( representationInfo->segmentBase, xmlElem, XML_TAG_SEGMENTBASE, MpegDashSegmentBaseInfo, B_PlaybackIp_MpegDashCreateSegmentBase, RepresentationCreateError );

    /* Create the SegmentList subelement (if there is one) */
    MPEGDASH_CREATE_SUBELEMT( representationInfo->segmentList, xmlElem, XML_TAG_SEGMENTLIST, MpegDashSegmentListInfo, B_PlaybackIp_MpegDashCreateSegmentList, RepresentationCreateError );

    return representationInfo;

RepresentationCreateError:
    B_PlaybackIp_MpegDashDestroyRepresentation(representationInfo);
    return NULL;
}



/********************* Create/destroy "ContentComponentInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroyContentComponent(MpegDashContentComponentInfo *contentComponentInfo)
{
    /* Destroy subelements...*/
    /* No sublements yet. */

    /* Deallocate the ContentComponentInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( contentComponentInfo, MpegDashContentComponentInfo);
}

static MpegDashContentComponentInfo *
B_PlaybackIp_MpegDashCreateContentComponent(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement   xmlElem)
{
    MpegDashContentComponentInfo             * contentComponentInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( contentComponentInfo, xmlElem, MpegDashContentComponentInfo);

    contentComponentInfo->id                      = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_ID          );
    contentComponentInfo->lang                    = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_LANG        );
    contentComponentInfo->contentType             = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_CONTENTTYPE );
    contentComponentInfo->par                     = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_PAR         );

    /* Create subelements...*/

    /* GARYWASHERE: Also need to add the following subelements to the ContentComponent. */
/*  BLST_Q_HEAD(mpegDashAccessibilityInfoHead,    MpegDashAccessibilityInfo)    mpegDashAccessibilityInfoHead;  */
/*  BLST_Q_HEAD(mpegDashRoleInfoHead,             MpegDashRoleInfo)             mpegDashRoleInfoHead;           */
/*  BLST_Q_HEAD(mpegDashRatingInfoHead,           MpegDashRatingInfo)           mpegDashRatingInfoHead;         */
/*  BLST_Q_HEAD(mpegDashViewpointInfoHead,        MpegDashViewpointInfo)        mpegDashViewpointInfoHead;      */

    return contentComponentInfo;

#if 0  /* We'll need the following code when adding the subelements. */
    ContentComponentCreateError:
    B_PlaybackIp_MpegDashDestroyContentComponent(contentComponentInfo);
    return NULL;
#endif
}



/********************* Create/destroy "AdaptationSetInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroyAdaptationSet(MpegDashAdaptationSetInfo *adaptationSetInfo)
{
    /* Destroy subelements...*/
    /* Destroy Representations */
    MPEGDASH_DESTROY_SUBELEM_LIST(adaptationSetInfo->mpegDashRepresentationInfoHead, MpegDashRepresentationInfo, B_PlaybackIp_MpegDashDestroyRepresentation, nextRepresentationInfo);

    /* Destroy BaseUrl */
    MPEGDASH_DESTROY_SUBELEM_LIST(adaptationSetInfo->mpegDashBaseUrlInfoHead, MpegDashBaseUrlInfo, B_PlaybackIp_MpegDashDestroyBaseUrl, nextBaseUrlInfo);

    /* Destroy ContentComponents */
    MPEGDASH_DESTROY_SUBELEM_LIST(adaptationSetInfo->mpegDashContentComponentInfoHead, MpegDashContentComponentInfo, B_PlaybackIp_MpegDashDestroyContentComponent, nextContentComponentInfo);

    /* Destroy SegmentList (if there is one) */
    MPEGDASH_DESTROY_SUBELEM( adaptationSetInfo->segmentList, MpegDashSegmentListInfo, B_PlaybackIp_MpegDashDestroySegmentList);

    /* Destroy SegmentBase (if there is one) */
    MPEGDASH_DESTROY_SUBELEM( adaptationSetInfo->segmentBase, MpegDashSegmentBaseInfo, B_PlaybackIp_MpegDashDestroySegmentBase);

    /* Destroy SegmentTemplate (if there is one) */
    MPEGDASH_DESTROY_SUBELEM( adaptationSetInfo->segmentTemplate, MpegDashSegmentTemplateInfo, B_PlaybackIp_MpegDashDestroySegmentTemplate);

    /* Destroy the RepresentationBase base class objects */
    if (adaptationSetInfo->baseTypeRepresentationBase) {
        B_PlaybackIp_MpegDashDestroyRepresentationBase(adaptationSetInfo->baseTypeRepresentationBase);
    }

    /* Deallocate the AdaptationSetInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( adaptationSetInfo, MpegDashAdaptationSetInfo);
}

static MpegDashAdaptationSetInfo *
B_PlaybackIp_MpegDashCreateAdaptationSet(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement   xmlElem)
{
    MpegDashAdaptationSetInfo             * adaptationSetInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( adaptationSetInfo, xmlElem, MpegDashAdaptationSetInfo);

    /* First, get the fields for the RepresentationBase structure. */
    adaptationSetInfo->baseTypeRepresentationBase = B_PlaybackIp_MpegDashCreateRepresentationBase(playback_ip, xmlElem);

    adaptationSetInfo->href                    = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_HREF                   );
    adaptationSetInfo->actuate                 = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_ACTUATE                );
    adaptationSetInfo->id                      = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_ID                     );
    adaptationSetInfo->group                   = B_PlaybackIp_XmlElem_FindAttrValueUnsigned(xmlElem, XML_ATT_GROUP              , 0 );
    adaptationSetInfo->lang                    = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_LANG                   );
    adaptationSetInfo->contentType             = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_CONTENTTYPE            );
    adaptationSetInfo->par                     = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_PAR                    );
    adaptationSetInfo->minBandwidth            = B_PlaybackIp_XmlElem_FindAttrValueUnsigned(xmlElem, XML_ATT_MINBANDWIDTH       , 0 );
    adaptationSetInfo->maxBandwidth            = B_PlaybackIp_XmlElem_FindAttrValueUnsigned(xmlElem, XML_ATT_MAXBANDWIDTH       , 0 );
    adaptationSetInfo->minWidth                = B_PlaybackIp_XmlElem_FindAttrValueUnsigned(xmlElem, XML_ATT_MINWIDTH           , 0 );
    adaptationSetInfo->maxWidth                = B_PlaybackIp_XmlElem_FindAttrValueUnsigned(xmlElem, XML_ATT_MAXWIDTH           , 0 );
    adaptationSetInfo->minHeight               = B_PlaybackIp_XmlElem_FindAttrValueUnsigned(xmlElem, XML_ATT_MINHEIGHT          , 0 );
    adaptationSetInfo->maxHeight               = B_PlaybackIp_XmlElem_FindAttrValueUnsigned(xmlElem, XML_ATT_MAXHEIGHT          , 0 );
    adaptationSetInfo->minFrameRate            = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_MINFRAMERATE           );
    adaptationSetInfo->maxFrameRate            = B_PlaybackIp_XmlElem_FindAttrValue(        xmlElem, XML_ATT_MAXFRAMERATE           );
    adaptationSetInfo->segmentAlignment        = B_PlaybackIp_XmlElem_FindAttrValueBoolean( xmlElem, XML_ATT_SEGMENTALIGNMENT    , false );
    adaptationSetInfo->subsegmentAlignment     = B_PlaybackIp_XmlElem_FindAttrValueBoolean( xmlElem, XML_ATT_SUBSEGMENTALIGNMENT , false );
    adaptationSetInfo->subsegmentStartsWithSAP = B_PlaybackIp_XmlElem_FindAttrValueUnsigned(xmlElem, XML_ATT_SUBSEGMENTSTARTSWITHSAP , 0 );
    adaptationSetInfo->bitstreamSwitching      = B_PlaybackIp_XmlElem_FindAttrValueBoolean( xmlElem, XML_ATT_BITSTREAMSWITCHING  , false );

    /* Create subelements...*/

    /* GARYWASHERE: Also need to add the following subelements to the AdaptationSet. */
/*  BLST_Q_HEAD(mpegDashAccessibilityInfoHead,    MpegDashAccessibilityInfo)    mpegDashAccessibilityInfoHead;  */
/*  BLST_Q_HEAD(mpegDashRoleInfoHead,             MpegDashRoleInfo)             mpegDashRoleInfoHead;           */
/*  BLST_Q_HEAD(mpegDashRatingInfoHead,           MpegDashRatingInfo)           mpegDashRatingInfoHead;         */
/*  BLST_Q_HEAD(mpegDashViewpointInfoHead,        MpegDashViewpointInfo)        mpegDashViewpointInfoHead;      */

    /* Create the SegmentTemplate subelement (if there is one) */
    MPEGDASH_CREATE_SUBELEMT( adaptationSetInfo->segmentTemplate, xmlElem, XML_TAG_SEGMENTTEMPLATE, MpegDashSegmentTemplateInfo, B_PlaybackIp_MpegDashCreateSegmentTemplate, AdaptationSetCreateError );

    /* Create the SegmentBase subelement (if there is one) */
    MPEGDASH_CREATE_SUBELEMT( adaptationSetInfo->segmentBase, xmlElem, XML_TAG_SEGMENTBASE, MpegDashSegmentBaseInfo, B_PlaybackIp_MpegDashCreateSegmentBase, AdaptationSetCreateError );

    /* Create the SegmentList subelement (if there is one) */
    MPEGDASH_CREATE_SUBELEMT( adaptationSetInfo->segmentList, xmlElem, XML_TAG_SEGMENTLIST, MpegDashSegmentListInfo, B_PlaybackIp_MpegDashCreateSegmentList, AdaptationSetCreateError );

    /* Create ContentComponent Subelements */
    MPEGDASH_CREATE_SUBELEM_LIST( adaptationSetInfo->mpegDashContentComponentInfoHead, xmlElem, XML_TAG_CONTENTCOMPONENT, MpegDashContentComponentInfo, B_PlaybackIp_MpegDashCreateContentComponent, nextContentComponentInfo, AdaptationSetCreateError );

    /* Create BaseURL Subelements */
    MPEGDASH_CREATE_SUBELEM_LIST( adaptationSetInfo->mpegDashBaseUrlInfoHead,          xmlElem, XML_TAG_BASEURL,          MpegDashBaseUrlInfo,          B_PlaybackIp_MpegDashCreateBaseUrl,          nextBaseUrlInfo,          AdaptationSetCreateError );

    /* Create Representation Subelements */
    MPEGDASH_CREATE_SUBELEM_LIST( adaptationSetInfo->mpegDashRepresentationInfoHead,   xmlElem, XML_TAG_REPRESENTATION,   MpegDashRepresentationInfo,   B_PlaybackIp_MpegDashCreateRepresentation,   nextRepresentationInfo,   AdaptationSetCreateError );
    MPEGDASH_SET_SUBELEM_LIST_PARENT(adaptationSetInfo->mpegDashRepresentationInfoHead, MpegDashRepresentationInfo, adaptationSetInfo, nextRepresentationInfo);

    return adaptationSetInfo;

AdaptationSetCreateError:
    B_PlaybackIp_MpegDashDestroyAdaptationSet(adaptationSetInfo);
    return NULL;
}


/********************* Create/destroy "PeriodInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroyPeriod(MpegDashPeriodInfo *periodInfo)
{
    /* Destroy subelements...*/
    /* Destroy AdaptationSets */
    MPEGDASH_DESTROY_SUBELEM_LIST(periodInfo->mpegDashAdaptationSetInfoHead, MpegDashAdaptationSetInfo, B_PlaybackIp_MpegDashDestroyAdaptationSet, nextAdaptationSetInfo);

    /* Destroy SegmentList (if there is one) */
    MPEGDASH_DESTROY_SUBELEM( periodInfo->segmentList, MpegDashSegmentListInfo, B_PlaybackIp_MpegDashDestroySegmentList);

    /* Destroy SegmentBase (if there is one) */
    MPEGDASH_DESTROY_SUBELEM( periodInfo->segmentBase, MpegDashSegmentBaseInfo, B_PlaybackIp_MpegDashDestroySegmentBase);

    /* Destroy SegmentTemplate (if there is one) */
    MPEGDASH_DESTROY_SUBELEM( periodInfo->segmentTemplate, MpegDashSegmentTemplateInfo, B_PlaybackIp_MpegDashDestroySegmentTemplate);

    /* Destroy BaseUrl */
    MPEGDASH_DESTROY_SUBELEM_LIST(periodInfo->mpegDashBaseUrlInfoHead, MpegDashBaseUrlInfo, B_PlaybackIp_MpegDashDestroyBaseUrl, nextBaseUrlInfo);

    /* Deallocate the PeriodInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( periodInfo, MpegDashPeriodInfo);
}

static MpegDashPeriodInfo *
B_PlaybackIp_MpegDashCreatePeriod(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement   xmlElem)
{
    MpegDashPeriodInfo             * periodInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( periodInfo, xmlElem, MpegDashPeriodInfo);

    periodInfo->href                       = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_HREF               );
    periodInfo->actuate                    = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_ACTUATE            );
    periodInfo->id                         = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_ID                 );
    periodInfo->start                      = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_START              );
    periodInfo->duration                   = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_DURATION           );
    periodInfo->bitstreamSwitching         = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_BITSTREAMSWITCHING );

    /* Convert the DateTime and Duration attributes to their "cooked" internal
     * formats. */
    if (periodInfo->start) {
        B_PlaybackIp_MpegDashGetDurationFromXml(periodInfo->start, &periodInfo->cooked.start);
    }
    if (periodInfo->duration) {
        B_PlaybackIp_MpegDashGetDurationFromXml(periodInfo->duration, &periodInfo->cooked.duration);
    }

    /* Create subelements...*/
    /* Create BaseURL Subelements */
    MPEGDASH_CREATE_SUBELEM_LIST( periodInfo->mpegDashBaseUrlInfoHead,       xmlElem, XML_TAG_BASEURL,       MpegDashBaseUrlInfo,        B_PlaybackIp_MpegDashCreateBaseUrl,       nextBaseUrlInfo,       PeriodCreateError );

    /* Create the SegmentTemplate subelement (if there is one) */
    MPEGDASH_CREATE_SUBELEMT( periodInfo->segmentTemplate, xmlElem, XML_TAG_SEGMENTTEMPLATE, MpegDashSegmentTemplateInfo, B_PlaybackIp_MpegDashCreateSegmentTemplate, PeriodCreateError );

    /* Create the SegmentBase subelement (if there is one) */
    MPEGDASH_CREATE_SUBELEMT( periodInfo->segmentBase, xmlElem, XML_TAG_SEGMENTBASE, MpegDashSegmentBaseInfo, B_PlaybackIp_MpegDashCreateSegmentBase, PeriodCreateError );

    /* Create the SegmentList subelement (if there is one) */
    MPEGDASH_CREATE_SUBELEMT( periodInfo->segmentList, xmlElem, XML_TAG_SEGMENTLIST, MpegDashSegmentListInfo, B_PlaybackIp_MpegDashCreateSegmentList, PeriodCreateError );

    /* Create AdaptationSet Subelements */
    MPEGDASH_CREATE_SUBELEM_LIST( periodInfo->mpegDashAdaptationSetInfoHead, xmlElem, XML_TAG_ADAPTATIONSET,  MpegDashAdaptationSetInfo, B_PlaybackIp_MpegDashCreateAdaptationSet, nextAdaptationSetInfo, PeriodCreateError );
    MPEGDASH_SET_SUBELEM_LIST_PARENT(periodInfo->mpegDashAdaptationSetInfoHead, MpegDashAdaptationSetInfo, periodInfo, nextAdaptationSetInfo);

    /* GARYWASHERE: Also need to add the following subelements to the Period. */
/*  BLST_Q_HEAD(mpegDashSubsetInfoHead,          MpegDashSubsetInfo)          mpegDashSubsetInfoHead;       */

    /* Now that we've created the Period and all the objects below it, this is
     * probably a good time to inherit some attributes (integer and boolean)
     * from the Period or AdaptationSet and store them in the Representation for
     * ease of access.   */
    {
        MpegDashAdaptationSetInfo   * adaptationSetInfo;

        for(adaptationSetInfo = BLST_Q_FIRST(&periodInfo->mpegDashAdaptationSetInfoHead) ; adaptationSetInfo ; adaptationSetInfo = BLST_Q_NEXT(adaptationSetInfo, nextAdaptationSetInfo) )
        {
            MpegDashRepresentationInfo   * representationInfo;

            for(representationInfo = BLST_Q_FIRST(&adaptationSetInfo->mpegDashRepresentationInfoHead) ; representationInfo ; representationInfo = BLST_Q_NEXT(representationInfo, nextRepresentationInfo) )
            {

                MpegDashSegmentBaseInfo     * segmentBase     = MPEG_DASH_GET_INHERITED_SEGMENT_TYPE( representationInfo, segmentBase);
                MpegDashSegmentListInfo     * segmentList     = MPEG_DASH_GET_INHERITED_SEGMENT_TYPE( representationInfo, segmentList);
                MpegDashSegmentTemplateInfo * segmentTemplate = MPEG_DASH_GET_INHERITED_SEGMENT_TYPE( representationInfo, segmentTemplate);

                if (segmentBase)        /* Only a single segment in this kind of Representation. */
                {
                    const char    * pChar;

                    /* Inherit SegmentBase fields (timescale, presentationTimeOffset, indexRangeExact) to Representation.*/
                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentBase, timescale);
                    representationInfo->inheritedTimescale = pChar ? strtoul(pChar, NULL, 10) : 1; /* if "timescale" is not specified, default to 1 (Section 5.3.9.2.2) */

                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentBase, presentationTimeOffset);
                    representationInfo->inheritedPresentationTimeOffset = pChar ? strtoul(pChar, NULL, 10) : 0; /* if not specified, default to 0 (Section 5.3.9.2.2) */

                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentBase, indexRangeExact);
                    representationInfo->inheritedIndexRangeExact = (pChar && ((strcasecmp(pChar, "true") == 0) || (strcasecmp(pChar, "1") == 0))) ? true : false;

                    /* No MultipleSegmentBase fields for SegmentBase. */
                }

                if (segmentTemplate)    /* Segments are specified with a Segment Template. */
                {
                    const char    * pChar;

                    /* Inherit SegmentBase fields (timescale, presentationTimeOffset, indexRangeExact) to Representation.*/
                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentTemplate, baseTypeMultipleSegmentBase->baseTypeSegmentBase->timescale);
                    representationInfo->inheritedTimescale =   pChar ? strtoul(pChar, NULL, 10) : 1; /* if "timescale" is not specified, default to 1 (Section 5.3.9.2.2) */

                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentTemplate, baseTypeMultipleSegmentBase->baseTypeSegmentBase->presentationTimeOffset);
                    representationInfo->inheritedPresentationTimeOffset = pChar ? strtoul(pChar, NULL, 10) : 0; /* if not specified, default to 0 (Section 5.3.9.2.2) */

                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentTemplate, baseTypeMultipleSegmentBase->baseTypeSegmentBase->indexRangeExact);
                    representationInfo->inheritedIndexRangeExact = (pChar && ((strcasecmp(pChar, "true") == 0) || (strcasecmp(pChar, "1") == 0))) ? true : false;

                    /* Inherit MultipleSegmentBase fields (startNumber, duration) to Representation.*/
                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentTemplate, baseTypeMultipleSegmentBase->startNumber);
                    representationInfo->inheritedStartNumber = pChar ? strtoul(pChar, NULL, 10) : 1; /* if "startNumber" is not specified, default to 1 (Section 5.3.9.5.3) */

                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentTemplate, baseTypeMultipleSegmentBase->duration);
                    representationInfo->inheritedDuration =    pChar ? strtoul(pChar, NULL, 10) : 0; /* if "duration" is not specified, leave at zero */
                }

                if (segmentList)        /* Segments are specified with a Segment List. */
                {
                    const char    * pChar;

                    /* Inherit SegmentBase fields (timescale, presentationTimeOffset, indexRangeExact) to Representation.*/
                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentList, baseTypeMultipleSegmentBase->baseTypeSegmentBase->timescale);
                    representationInfo->inheritedTimescale =   pChar ? strtoul(pChar, NULL, 10) : 1; /* if "timescale" is not specified, default to 1 (Section 5.3.9.2.2) */

                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentList, baseTypeMultipleSegmentBase->baseTypeSegmentBase->presentationTimeOffset);
                    representationInfo->inheritedPresentationTimeOffset = pChar ? strtoul(pChar, NULL, 10) : 0; /* if not specified, default to 0 (Section 5.3.9.2.2) */

                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentList, baseTypeMultipleSegmentBase->baseTypeSegmentBase->indexRangeExact);
                    representationInfo->inheritedIndexRangeExact = (pChar && ((strcasecmp(pChar, "true") == 0) || (strcasecmp(pChar, "1") == 0))) ? true : false;

                    /* Inherit MultipleSegmentBase fields (startNumber, duration) to Representation.*/
                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentList, baseTypeMultipleSegmentBase->startNumber);
                    representationInfo->inheritedStartNumber = pChar ? strtoul(pChar, NULL, 10) : 1; /* if "startNumber" is not specified, default to 1 (Section 5.3.9.5.3) */

                    pChar = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationInfo, segmentList, baseTypeMultipleSegmentBase->duration);
                    representationInfo->inheritedDuration =    pChar ? strtoul(pChar, NULL, 10) : 0; /* if "duration" is not specified, leave at zero */
                }   /* End if Segment List */
            }       /* End do for each Representation */
        }           /* End do for each AdaptationSet */
    }

    return periodInfo;

PeriodCreateError:
    B_PlaybackIp_MpegDashDestroyPeriod(periodInfo);
    return NULL;
}


/********************* Create/destroy "MpdInfo" objects *******************/
static void B_PlaybackIp_MpegDashDestroyMpd(MpegDashMpdInfo *mpdInfo)
{
    /* Destroy subelements...*/

    /* Destroy Periods */
    MPEGDASH_DESTROY_SUBELEM_LIST(mpdInfo->mpegDashPeriodInfoHead, MpegDashPeriodInfo, B_PlaybackIp_MpegDashDestroyPeriod, nextPeriodInfo);

    /* Destroy Locations */
    MPEGDASH_DESTROY_SUBELEM_LIST(mpdInfo->mpegDashLocationInfoHead, MpegDashAnyTextType, B_PlaybackIp_MpegDashDestroyAnyText, nextAnyText);

    /* Destroy BaseUrls */
    MPEGDASH_DESTROY_SUBELEM_LIST(mpdInfo->mpegDashBaseUrlInfoHead, MpegDashBaseUrlInfo, B_PlaybackIp_MpegDashDestroyBaseUrl, nextBaseUrlInfo);

    /* Deallocate the MpdInfo structure. */
    MPEGDASH_DESTROY_DEALLOCATE( mpdInfo, MpegDashMpdInfo);
}


static MpegDashMpdInfo *
B_PlaybackIp_MpegDashCreateMpd(B_PlaybackIpHandle playback_ip, B_PlaybackIp_XmlElement   xmlElem)
{
    MpegDashMpdInfo             * mpdInfo;
    BSTD_UNUSED(playback_ip);

    MPEGDASH_CREATE_ALLOCATE( mpdInfo, xmlElem, MpegDashMpdInfo);

    mpdInfo->profiles                   = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_PROFILES                  );
    mpdInfo->id                         = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_ID                        );
    mpdInfo->type                       = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_TYPE                      );
    mpdInfo->availabilityStartTime      = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_AVAILABILITYSTARTTIME     );
    mpdInfo->availabilityEndTime        = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_AVAILABILITYENDTIME       );
    mpdInfo->mediaPresentationDuration  = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_MEDIAPRESENTATIONDURATION );
    mpdInfo->minimumUpdatePeriod        = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_MINIMUMUPDATEPERIOD       );
    mpdInfo->minBufferTime              = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_MINBUFFERTIME             );
    mpdInfo->timeShiftBufferDepth       = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_TIMESHIFTBUFFERDEPTH      );
    mpdInfo->suggestedPresentationDelay = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_SUGGESTEDPRESENTATIONDELAY);
    mpdInfo->maxSegmentDuration         = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_MAXSEGMENTDURATION        );
    mpdInfo->maxSubsegmentDuration      = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem , XML_ATT_MAXSUBSEGMENTDURATION     );

    /* Convert the DateTime and Duration attributes to their "cooked" internal
     * formats. */
    if (mpdInfo->availabilityStartTime) {
        B_PlaybackIp_MpegDashGetDateTimeFromXml(mpdInfo->availabilityStartTime, &mpdInfo->cooked.availabilityStartTime);
    }
    if (mpdInfo->availabilityEndTime) {
        B_PlaybackIp_MpegDashGetDateTimeFromXml(mpdInfo->availabilityEndTime, &mpdInfo->cooked.availabilityEndTime);
    }
    if (mpdInfo->mediaPresentationDuration) {
        B_PlaybackIp_MpegDashGetDurationFromXml(mpdInfo->mediaPresentationDuration, &mpdInfo->cooked.mediaPresentationDuration);
    }
    if (mpdInfo->minimumUpdatePeriod) {
        B_PlaybackIp_MpegDashGetDurationFromXml(mpdInfo->minimumUpdatePeriod, &mpdInfo->cooked.minimumUpdatePeriod);
    }
    if (mpdInfo->minBufferTime) {
        B_PlaybackIp_MpegDashGetDurationFromXml(mpdInfo->minBufferTime, &mpdInfo->cooked.minBufferTime);
    }
    if (mpdInfo->timeShiftBufferDepth) {
        B_PlaybackIp_MpegDashGetDurationFromXml(mpdInfo->timeShiftBufferDepth, &mpdInfo->cooked.timeShiftBufferDepth);
    }
    if (mpdInfo->suggestedPresentationDelay) {
        B_PlaybackIp_MpegDashGetDurationFromXml(mpdInfo->suggestedPresentationDelay, &mpdInfo->cooked.suggestedPresentationDelay);
    }
    if (mpdInfo->maxSegmentDuration) {
        B_PlaybackIp_MpegDashGetDurationFromXml(mpdInfo->maxSegmentDuration, &mpdInfo->cooked.maxSegmentDuration);
    }
    if (mpdInfo->maxSubsegmentDuration) {
        B_PlaybackIp_MpegDashGetDurationFromXml(mpdInfo->maxSubsegmentDuration, &mpdInfo->cooked.maxSubsegmentDuration);
    }

    /* Create subelements...*/
    MPEGDASH_CREATE_SUBELEM_LIST( mpdInfo->mpegDashBaseUrlInfoHead,  xmlElem, XML_TAG_BASEURL,  MpegDashBaseUrlInfo, B_PlaybackIp_MpegDashCreateBaseUrl, nextBaseUrlInfo, MpdCreateError );

    MPEGDASH_CREATE_SUBELEM_LIST( mpdInfo->mpegDashLocationInfoHead, xmlElem, XML_TAG_LOCATION, MpegDashAnyTextType, B_PlaybackIp_MpegDashCreateAnyText, nextAnyText, MpdCreateError );

    MPEGDASH_CREATE_SUBELEM_LIST( mpdInfo->mpegDashPeriodInfoHead,   xmlElem, XML_TAG_PERIOD,   MpegDashPeriodInfo,  B_PlaybackIp_MpegDashCreatePeriod,  nextPeriodInfo,  MpdCreateError );
    MPEGDASH_SET_SUBELEM_LIST_PARENT(mpdInfo->mpegDashPeriodInfoHead, MpegDashPeriodInfo, mpdInfo, nextPeriodInfo);

    /* Set the "isDynamic" flag if appropriate. */
    if (strcmp(mpdInfo->type, "dynamic") == 0)
    {
        mpdInfo->isDynamic = true;
    }

    return mpdInfo;

MpdCreateError:
    B_PlaybackIp_MpegDashDestroyMpd(mpdInfo);
    return NULL;
}

/*****************************************************************************
 * Perform template-based URL substitution for a given non-numeric identifier.
 * Given a text string, just copy it into the callers buffer.
 *****************************************************************************/
static const char *
B_PlaybackIp_MpegDashTemplateStrcpy(
    const char      * pToken,       /* points to starting "$" of current token in template string. */
    char           ** pResult,      /* points to the result buffer to receive the converted value. */
    int             * resultLength, /* length of remaining result buffer. */
    const char      * str )         /* string to be copied. */
{
    int count       = 0;            /* length of string to be copied. */
    const char *  pAfterToken = pToken;

    /* We should be at the start of an identifier token. Start by
     * skipping to the next delimiter characters. */
    while (true) {
        pAfterToken++;
        if (*pAfterToken == '\0') goto error;      /* end of string... shouldn't happen! */
        if (*pAfterToken == '$') break;            /* end of identifier => no format string */
    }
    pAfterToken++;                                  /* move past the token's trailing '$' */

    count = strlen(str);
    if (count >= *resultLength) {
        BDBG_ERR(("%s: URL format buffer overflow for token: %s", __FUNCTION__, pToken));
        goto error;
    }

    strncpy(*pResult, str, count);
    *pResult += count;
    *resultLength -= count;

    return pAfterToken;    /* return pointer to first byte after token. */

error:
    return NULL;
}

/*****************************************************************************
 * Perform template-based URL substitution for a given Identifier.  Given a
 * binary value, format it according to its corresponding $<Identifier>$ and
 * put the result into the callers buffer.
 *****************************************************************************/
static const char *
B_PlaybackIp_MpegDashTemplatePrintf(
    const char      * pToken,       /* points to starting "$" of current token in template string. */
    char           ** pResult,      /* points to the result buffer to receive the converted value. */
    int             * resultLength, /* length of remaining result buffer. */
    int               value )       /* current binary value to be converted. */
{
    int           fieldWidth  = 1;   /* Minimum field width specified by format tag. */
    unsigned long count       = 0;   /* Minimum field width determined by field value. */
    unsigned long tokenLength = 0;
    const char *  pFmt = pToken;

    /* We should be at the start of an identifier token. Start by
     * skipping to the next delimiter characters. */
    while (true) {
        pFmt++;
        if (*pFmt == '\0') goto error;      /* end of string... shouldn't happen! */
        if (*pFmt == '%') break;            /* start of format string */
        if (*pFmt == '$') break;            /* end of identifier => no format string */
    }

    /* If we are at the start of a "%0[width]d" format string,
     * convert the "width" to binary so we know the minimum
     * numbers of characters to use for the converted token.*/
    if (*pFmt == '%') {
        fieldWidth = 0;
        if (*(++pFmt) != '0') goto error;
        pFmt++;
        while (isdigit(*pFmt)) {
            fieldWidth *= 10;
            fieldWidth += *pFmt - '0';
            pFmt++;
        }
        if (*pFmt != 'd') goto error;
        ++pFmt;     /* point to first char after the 'd, should be a '$'*/
    }

    if (*pFmt++ != '$') goto error;

    tokenLength = pFmt - pToken;

    count = snprintf(*pResult, *resultLength,"%0*u", fieldWidth, value);

    *pResult += count;
    *resultLength -= count;

    return pToken + tokenLength;    /* return pointer to first byte after token. */

error:
    return NULL;
}


/*****************************************************************************
 * For template-based URL substitution, determine the number of characters that
 * a given Identifier's formatted value will consume.  This is required to
 * determine what size of result buffer to allocate.
 *****************************************************************************/
static const char *
B_PlaybackIp_MpegDashAdjustTemplateResultLength(
    const char      * pToken,   /* points to starting "$" of current token in template string. */
    unsigned int      value,    /* the value associated with the token (seg number, bandwidth, or time). */
    int             * pResultLength )    /* current result length to be adjusted . */
{
    unsigned long fieldWidth  = 0;   /* Minimum field width specified by format tag. */
    unsigned long valueWidth  = 0;   /* Minimum field width determined by field value. */
    unsigned long tokenLength = 0;
    const char  * pFmt = pToken;

    /* We should be at the start of an identifier token. Start by
     * skipping to the next delimiter characters. */
    while (true) {
        pFmt++;
        if (*pFmt == '\0') goto error;
        if (*pFmt == '%') break;
        if (*pFmt == '$') break;
    }

    /* If we are at the start of a "%0[width]d" format string,
     * convert the "width" to binary so we know the minimum
     * numbers of characters to use for the converted token.*/
    if (*pFmt == '%') {
        if (*(++pFmt) != '0') goto error;
        pFmt++;
        while (isdigit(*pFmt)) {
            fieldWidth *= 10;
            fieldWidth += *pFmt - '0';
            pFmt++;
        }
        if (*pFmt != 'd') goto error;
        ++pFmt;
    }

    if (*pFmt++ != '$') goto error;

    tokenLength = pFmt - pToken;

    /* Determine width of the replacement string */

    /* To find the length of the "$Number$" replacement, we need
       to count how many digits the number will take. */
    {
        int digits = 0;
        unsigned num = value;

        while (num > 0) {
            digits++;
            num /= 10;
        }
        if (digits == 0) digits = 1;

        valueWidth = digits;
    }
    *pResultLength -= tokenLength;
    *pResultLength += valueWidth > fieldWidth ? valueWidth : fieldWidth;

    return pToken + tokenLength;

error:
    return NULL;
}


/*****************************************************************************
 * Given a template (from a SegmentTemplate element) replace each identifier
 * (token), delimited by "$", with its actual value.
 *****************************************************************************/
static char *
B_PlaybackIp_MpegDashCreateUrlFromTemplate(
    B_PlaybackIpHandle playback_ip,
    const char * templateStr,
    const char * representationId,
    unsigned     number,
    unsigned     bandwidthNumeric,
    unsigned long  time )
{
    BSTD_UNUSED(playback_ip);
    const char * pChar;

    const char * repToken       = "$RepresentationID$";
    const char * numToken       = "$Number";
    const char * bwToken        = "$Bandwidth";
    const char * timeToken      = "$Time";
    const char * dollarToken    = "$$";

    int    resultLength;
    char * resultString = NULL;
    char * pResultChar;
    char * pDelim;


    resultLength = strlen(templateStr) + 1;    /* Add 1 for null terminator. */

    /* First, we need to figure out how long the resulting string will be
     * (so we know how much to malloc).  Start by counting the number
     * of each type of identifier token that we have.
     */
    pChar = templateStr;
    while ( (pChar = strchr(pChar, '$')) != NULL) {     /* Point to the first/next token */

        if (strstr(pChar, repToken)== pChar) {
            unsigned int myTokenLength = strlen(repToken);
            resultLength -= myTokenLength;
            resultLength += strlen(representationId);
            pChar += myTokenLength;
        }
        else if (strstr(pChar, numToken)== pChar) {
            const char * pNext;
            pNext = B_PlaybackIp_MpegDashAdjustTemplateResultLength(pChar, number, &resultLength);
            if (!pNext) goto error;

            pChar = pNext;
        }
        else if (strstr(pChar, bwToken)== pChar) {
            const char * pNext;
            pNext = B_PlaybackIp_MpegDashAdjustTemplateResultLength(pChar, bandwidthNumeric, &resultLength);
            if (!pNext) goto error;

            pChar = pNext;
        }
        else if (strstr(pChar, timeToken)== pChar) {
            const char * pNext;
            pNext = B_PlaybackIp_MpegDashAdjustTemplateResultLength(pChar, time, &resultLength);
            if (!pNext) goto error;

            pChar = pNext;
        }
        else if (strstr(pChar, dollarToken)== pChar) {
            unsigned int myTokenLength = strlen(dollarToken);
            resultLength -= myTokenLength;
            resultLength += 1;  /* 1 == strlen("$") */
            pChar += myTokenLength;
        }
        else
        {
            BDBG_ERR(("%s: Unrecognized token in template: \"%s\"", __FUNCTION__, pChar));
            goto error;
        }
    }

    /* Now that we know the length of the final result string, go ahead
     * and malloc it. */
    resultString = (char *)BKNI_Malloc(resultLength);
    if (resultString == NULL) {
        BDBG_ERR(("%s: Failed to allocate %d bytes of memory for template substitution uri", __FUNCTION__, resultLength));
        goto error;
    }

    /* Now go through the template string again, but this time
     * copy the template string to the result string, but
     * replace each identifier token with it's replacement. */
    pChar = templateStr;
    pResultChar = resultString;


    while ( (pDelim = strchr(pChar, '$')) != NULL) {

        /* If there's anything before the first/next token, just
         * copy it straight to the result buffer. */
        int fragmentLength = pDelim - pChar;

        if (fragmentLength > 0) {
            strncpy(pResultChar, pChar, resultLength);
            pChar        += fragmentLength;
            pResultChar  += fragmentLength;
            resultLength -= fragmentLength;
        }

        /* Now we're at the first/next token, so we have to put the
         * token's replacement string into the result buffer. */
        if (strstr(pChar, repToken)== pChar) {
            const char * pNext;

            pNext = B_PlaybackIp_MpegDashTemplateStrcpy(pChar, &pResultChar, &resultLength, representationId);
            if (!pNext) goto error;

            pChar = pNext;
        }
        else if (strstr(pChar, numToken)== pChar) {
            const char * pNext;

            pNext = B_PlaybackIp_MpegDashTemplatePrintf(pChar, &pResultChar, &resultLength, number);
            if (!pNext) goto error;

            pChar = pNext;
        }
        else if (strstr(pChar, bwToken)== pChar) {
            const char * pNext;

            pNext = B_PlaybackIp_MpegDashTemplatePrintf(pChar, &pResultChar, &resultLength, bandwidthNumeric);
            if (!pNext) goto error;

            pChar = pNext;
        }
        else if (strstr(pChar, timeToken)== pChar) {
            const char * pNext;

            pNext = B_PlaybackIp_MpegDashTemplatePrintf(pChar, &pResultChar, &resultLength, time);
            if (!pNext) goto error;

            pChar = pNext;
        }
        else if (strstr(pChar, dollarToken)== pChar) {
            const char * pNext;

            pNext = B_PlaybackIp_MpegDashTemplateStrcpy(pChar, &pResultChar, &resultLength, "$");
            if (!pNext) goto error;

            pChar = pNext;
        }
        else
        {
            BDBG_ERR(("%s: Unrecognized token in template: \"%s\"", __FUNCTION__, templateStr));
            goto error;
        }
    } /* end while */

    /* Now we've finished with the last token, now take care of anything
       that might be after the last token.  */
    {
        int fragmentLength = strlen(pChar);
        if (fragmentLength >= resultLength) {
            BDBG_ERR(("%s: URL format buffer overflow for fragment: %s", __FUNCTION__, pChar));
            goto error;
        }
        else if (fragmentLength > 0) {
            strncpy(pResultChar, pChar, fragmentLength);
            pChar       += fragmentLength;
            pResultChar += fragmentLength;
            resultLength -= fragmentLength;
        }
    }

    /* Finally, just add the NULL-terminator and we're done.  */
    *pResultChar = '\0';
    resultLength -= 1;

    if (resultLength != 0) {
        BDBG_ERR(("%s: Unexpected non-zero resultLength=%d for URL template: %s", __FUNCTION__, resultLength, templateStr));
    }

    return resultString;

error:
    if (resultString) BKNI_Free(resultString);
    return NULL;
}


/*****************************************************************************
 * Convert a partial URL into an absolute URL, This involves prefixing with
 * a "baseURL", that might be specified in either the Representation,
 * AdaptationSet, Period, or the MPD.  And if no baseUrl is specified, then
 * we can prefix with the server and port that were used to download the
 * original MPD file.
 *****************************************************************************/
static char *
B_PlaybackIp_MpegDashCreateAbsoluteUrl(B_PlaybackIpHandle playback_ip, MpegDashRepresentationInfo * representationInfo, char * url)
{
    MpegDashSessionState        * mpegDashSession = playback_ip->mpegDashSessionState;
    B_PlaybackIpProtocol  protocol;
    char               * server = NULL;     /* Need to be freed on return. */
    char               * partialUri;
    unsigned             port;
    int                  returnCode;

    /* If the resulting URI is not absolute, look for a BaseUrl to prepend to it.   Look for the BaseUrl in
     * the Representation, then the AdaptationSet, then Period, then the MPD.
     */
    if (!http_absolute_uri(url)) {
        MpegDashBaseUrlInfo        * baseUrl;

        PRINTMSG_URL(("%s:%d: Non-absolute URI, looking for baseUrl", __FUNCTION__, __LINE__));

        /* Look for a BaseUrl in the Representation. */
        baseUrl = BLST_Q_FIRST(&representationInfo->mpegDashBaseUrlInfoHead);
        if (baseUrl)   PRINTMSG_URL(("%s:%d: Found baseUrl in Representation: %s", __FUNCTION__, __LINE__, baseUrl->childData));

        if (!baseUrl) {
            MpegDashAdaptationSetInfo   * adaptationSetInfo = representationInfo->parentObj;

            /* Look for a BaseUrl in the AdaptationSet. */
            baseUrl = BLST_Q_FIRST(&adaptationSetInfo->mpegDashBaseUrlInfoHead);
            if (baseUrl)   PRINTMSG_URL(("%s:%d: Found baseUrl in AdaptationSet: %s", __FUNCTION__, __LINE__, baseUrl->childData));

            if (!baseUrl) {
                MpegDashPeriodInfo      * periodInfo = adaptationSetInfo->parentObj;

                /* Look for a BaseUrl in the Period. */
                baseUrl = BLST_Q_FIRST(&periodInfo->mpegDashBaseUrlInfoHead);
                if (baseUrl)   PRINTMSG_URL(("%s:%d: Found baseUrl in Period: %s", __FUNCTION__, __LINE__, baseUrl->childData));

                if (!baseUrl) {
                    MpegDashMpdInfo      * mpdInfo = periodInfo->parentObj;

                    /* Look for a BaseUrl in the MPD. */
                    baseUrl = BLST_Q_FIRST(&mpdInfo->mpegDashBaseUrlInfoHead);
                    PRINTMSG_URL(("%s:%d: baseUrl from MPD: %p", __FUNCTION__, __LINE__, (void *)baseUrl));
                    if (baseUrl)   PRINTMSG_URL(("%s:%d: Found baseUrl in MPD: %s", __FUNCTION__, __LINE__, baseUrl->childData));
                }
            }
        }

        if (!baseUrl)   PRINTMSG_URL(("%s:%d: No baseUrl specified", __FUNCTION__, __LINE__));


        /* If we found a BaseUrl, apply it to the current URI. */
        if (baseUrl) {
            char *  newUri;

            /* build complete url using baseUrl. */

            PRINTMSG_URL(("%s:%d: Found BaseUrl=%p", __FUNCTION__, __LINE__, (void *)baseUrl));
            PRINTMSG_URL(("%s:%d: baseUrl->byteRange=%s", __FUNCTION__, __LINE__, baseUrl->byteRange));
            PRINTMSG_URL(("%s:%d: baseUrl->serviceLocation=%s", __FUNCTION__, __LINE__, baseUrl->serviceLocation));
            PRINTMSG_URL(("%s:%d: baseUrl->childData=%s", __FUNCTION__, __LINE__, baseUrl->childData));

            if (baseUrl->childData) {
                returnCode = mpegDash_parse_absolute_url(&protocol, &server, &port, &partialUri, (char *)baseUrl->childData);
                if (returnCode == false) {
                    BDBG_ERR(("%s:%d Failed to parse BaseUrl: %s", __FUNCTION__, __LINE__, baseUrl->childData ));
                    goto error;
                }
                if (protocol != B_PlaybackIpProtocol_eHttp && protocol != B_PlaybackIpProtocol_eHttps) {
                    BDBG_ERR(("%s:%d BaseUrl has non-HTTP protocol: %s", __FUNCTION__, __LINE__, baseUrl->childData));
                    goto error;
                }

                PRINTMSG_URL(("%s: Using BaseUrl: %s", __FUNCTION__, baseUrl->childData ));
                if ((newUri = B_PlaybackIp_MpegDashBuildAbsoluteUri(server, port, partialUri,  url)) == NULL) {
                    BDBG_ERR(("Failed to build URI at %s:%d", __FUNCTION__, __LINE__));
                    goto error;
                }
                BKNI_Free(url);
                url = newUri;

                PRINTMSG_URL(("%s: Used BaseUrl to build Absolute Uri: %s", __FUNCTION__, url ));
            }
        }

        /* If we still have a relative url, build complete url using server ip address & port # */
        if (!http_absolute_uri( url)) {
            char *  newUri;

            /* relative url, build complete url using server ip address & port # */
            PRINTMSG_URL(("%s: media segment URI is not absolute URI, using server and port from MPD URL.", __FUNCTION__));
            if ((newUri = B_PlaybackIp_MpegDashBuildAbsoluteUri(mpegDashSession->server, mpegDashSession->port, mpegDashSession->uri, url)) == NULL) {
                BDBG_ERR(("Failed to build URI at %s:%d", __FUNCTION__, __LINE__));
                goto error;
            }
            BKNI_Free(url);
            url = newUri;

            PRINTMSG_URL(("%s: Used MPD server/port/url to build Absolute Uri: %s", __FUNCTION__, url ));
        }
    }

    if (server) BKNI_Free(server);

    return url;

error:
    if (server) BKNI_Free(server);

    return NULL;
}


/*****************************************************************************
 * Given a RepresentationSeekContext, generate a URL to the requested kind
 * (media, initialization) of segment.
 *****************************************************************************/
static char *
B_PlaybackIp_MpegDashCreateAbsoluteUrlForSeek(B_PlaybackIpHandle playback_ip, MpegDashRepresentationSeekContext *representationSeekCtx, MpegDashUrlKind urlKind)
{
    char * url = NULL;

    MpegDashSegmentBaseInfo      * segmentBase;
    MpegDashSegmentListInfo      * segmentList;
    MpegDashSegmentTemplateInfo  * segmentTemplate;

    segmentBase     = representationSeekCtx->segmentBase;
    segmentList     = representationSeekCtx->segmentList;
    segmentTemplate = representationSeekCtx->segmentTemplate;

    if (segmentBase) {

        /* GARYWASHERE: Add handling for SegmentBase segments. */

        BDBG_ERR(("%s: Can't handle SegmentBase segments yet!", __FUNCTION__));
        goto error;
    }
    else if (segmentList) {

        const char * listUrl = NULL;

        /* GARYWASHERE: Review this for completeness. */

        if (urlKind == MPEG_DASH_URL_KIND_MEDIA) {
            if (representationSeekCtx->segmentUrlInfo) {
                listUrl = representationSeekCtx->segmentUrlInfo->media;
            }
        }
        else if (urlKind == MPEG_DASH_URL_KIND_SEGMENTINDEX) {
            listUrl = representationSeekCtx->segmentUrlInfo->index;
        }
        else if (urlKind == MPEG_DASH_URL_KIND_INITIALIZATION) {
            MpegDashUrlInfo  * initializationUrl = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationSeekCtx->representationInfo, segmentList, baseTypeMultipleSegmentBase->baseTypeSegmentBase->initialization);
            if (initializationUrl) {
                listUrl = initializationUrl->sourceURL;
            }
        }
        else if (urlKind == MPEG_DASH_URL_KIND_BITSTREAMSWITCHING) {
            listUrl = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationSeekCtx->representationInfo, segmentTemplate, bitstreamSwitching);
        }
        else if (urlKind == MPEG_DASH_URL_KIND_REPRESENTATIONINDEX) {
            MpegDashUrlInfo  * representationIndex = segmentTemplate->baseTypeMultipleSegmentBase->baseTypeSegmentBase->representationIndex;
            if (representationIndex) {
                listUrl = representationIndex->sourceURL;
            }
        }
        else
        {
            BDBG_ERR(("%s: Invalid urlKind: %u", __FUNCTION__, urlKind));
            goto error;
        }

        if (listUrl) {
            /* Start by building a URI by substituting the appropriate values into the template. */
            url = B_PlaybackIp_MpegDashCreateUrlFromTemplate(playback_ip,
                                                             listUrl,
                                                             "",
                                                             0,
                                                             0,
                                                             0);

            PRINTMSG_URL(("%s:%d: After template substitution: url=%s", __FUNCTION__, __LINE__, url));

            url = B_PlaybackIp_MpegDashCreateAbsoluteUrl(playback_ip, representationSeekCtx->representationInfo,  url);

            PRINTMSG_URL(("%s:%d: After making absolute: url=%s", __FUNCTION__, __LINE__, url));
        }

    }
    else if (segmentTemplate) {

        const char * templateUrl = NULL;
        unsigned segmentNumber = 0;

        if (urlKind == MPEG_DASH_URL_KIND_MEDIA) {
            templateUrl = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationSeekCtx->representationInfo, segmentTemplate, media);
            segmentNumber = representationSeekCtx->segmentNumber;
        }
        else if (urlKind == MPEG_DASH_URL_KIND_SEGMENTINDEX) {
            templateUrl = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationSeekCtx->representationInfo, segmentTemplate, index);
            segmentNumber = representationSeekCtx->segmentNumber;
        }
        else if (urlKind == MPEG_DASH_URL_KIND_INITIALIZATION) {
            templateUrl = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationSeekCtx->representationInfo, segmentTemplate, initialization);

            if(!templateUrl) {
                MpegDashUrlInfo  * initializationUrl = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationSeekCtx->representationInfo, segmentTemplate, baseTypeMultipleSegmentBase->baseTypeSegmentBase->initialization);
                if (initializationUrl) {
                    templateUrl = initializationUrl->sourceURL;
                }
            }
        }
        else if (urlKind == MPEG_DASH_URL_KIND_BITSTREAMSWITCHING) {
            templateUrl = MPEG_DASH_GET_INHERITED_SEGMENT_PROP(representationSeekCtx->representationInfo, segmentTemplate, bitstreamSwitching);
        }
        else if (urlKind == MPEG_DASH_URL_KIND_REPRESENTATIONINDEX) {
            MpegDashUrlInfo  * representationIndex = segmentTemplate->baseTypeMultipleSegmentBase->baseTypeSegmentBase->representationIndex;
            if (representationIndex) {
                templateUrl = representationIndex->sourceURL;
            }
        }
        else
        {
            BDBG_ERR(("%s: Invalid urlKind: %u", __FUNCTION__, urlKind));
            goto error;
        }

        /* If the URL hasn't been specified, just use a zero-length string.  This
         * will lead to using the BaseUrl as the requested URL.
         */
        if (urlKind==MPEG_DASH_URL_KIND_MEDIA &&  templateUrl == NULL)  templateUrl = "";

        if (templateUrl) {
            /* Start by building a URI by substituting the appropriate values into the template. */
            PRINTMSG_URL(("%s:%d: TemplateUrl=%s", __FUNCTION__, __LINE__, templateUrl));

            url = B_PlaybackIp_MpegDashCreateUrlFromTemplate(playback_ip,
                                                             templateUrl,
                                                             representationSeekCtx->representationId,
                                                             segmentNumber,
                                                             representationSeekCtx->representationInfo->bandwidthNumeric,
                                                             representationSeekCtx->segmentTimelineTime);

            PRINTMSG_URL(("%s:%d: After template substitution: url=%s", __FUNCTION__, __LINE__, url));

            url = B_PlaybackIp_MpegDashCreateAbsoluteUrl(playback_ip, representationSeekCtx->representationInfo,  url);

            PRINTMSG_URL(("%s:%d: After making absolute: url=%s", __FUNCTION__, __LINE__, url));
        }
    }
    else
    {
        BDBG_ERR(("%s: Representation has no segmentBase, segmentList, or segmentTemplate!", __FUNCTION__));
        goto error;
    }

    return url;

error:
    /* GARYWASHERE:  if (url) destroy url */
    return NULL;
}


/*****************************************************************************
 * Destroy and deallocate a MediaFileSegmentInfo structure.
 *****************************************************************************/
static void
B_PlaybackIp_MpegDashFreeMediaFileSegmentInfo(MediaFileSegmentInfo *mediaFileSegmentInfo)
{
    if (mediaFileSegmentInfo->absoluteUri) {
        BKNI_Free(mediaFileSegmentInfo->absoluteUri);
        mediaFileSegmentInfo->absoluteUri = NULL;
    }
    if (mediaFileSegmentInfo->server) {
        BKNI_Free(mediaFileSegmentInfo->server);
        mediaFileSegmentInfo->server = NULL;
    }
    PRINTMSG_DOWNLOAD(("%s: freed mediaFileSegmentInfo entry %p", __FUNCTION__, (void *)mediaFileSegmentInfo));
    BKNI_Free(mediaFileSegmentInfo);
}


/*****************************************************************************
 * Create a MediaFileSegmentInfo struct from a seek context.
 *****************************************************************************/
static MediaFileSegmentInfo *
B_PlaybackIp_MpegDashCreateMediaFileSegmentInfoFromSeek(B_PlaybackIpHandle playback_ip, MpegDashRepresentationSeekContext  * representationSeekCtx, MpegDashUrlKind urlKind)
{
    MediaFileSegmentInfo     *mediaFileSegmentInfo = NULL;

    /* If the seek is at the end, then we can't build a MFSI
     * struct for it, so just return NULL.  */
    if (representationSeekCtx->isAtEof) {
        goto error;
    }

    /* Create a MediaFileSegmentInfo struct using the info from the requestSeekCtx. */
    mediaFileSegmentInfo = (MediaFileSegmentInfo *)BKNI_Malloc(sizeof(MediaFileSegmentInfo));
    if (mediaFileSegmentInfo  == NULL) {
        BDBG_ERR(("%s: Failed to allocate %d bytes of memory for media segment info structure", __FUNCTION__, (int)sizeof(MediaFileSegmentInfo)));
        goto error;
    }

    PRINTMSG_SEEK(("%s: allocated %d bytes of memory for media segment info: %p", __FUNCTION__, (int)sizeof(MediaFileSegmentInfo), (void *)mediaFileSegmentInfo));

    memset(mediaFileSegmentInfo, 0, sizeof(MediaFileSegmentInfo));

    switch (urlKind) {
        /* Handle the Segment-specific urlKinds (media, segment index). */
        case MPEG_DASH_URL_KIND_MEDIA:
        case MPEG_DASH_URL_KIND_SEGMENTINDEX:

            mediaFileSegmentInfo->mediaSequence = representationSeekCtx->segmentNumber;

            /* Convert duration from timescale units to milliseconds. */
            mediaFileSegmentInfo->duration = (representationSeekCtx->durationInTimescaleUnits * 1000) / representationSeekCtx->timescale;

            PRINTMSG_SEEK(("%s:%d: mediaFileSegmentInfo->mediaSequence=%d", __FUNCTION__, __LINE__, mediaFileSegmentInfo->mediaSequence));
            PRINTMSG_SEEK(("%s:%d: mediaFileSegmentInfo->duration=%lu", __FUNCTION__, __LINE__, mediaFileSegmentInfo->duration));

            break;

    default:
        break;
    }

    mediaFileSegmentInfo->hasAudio = representationSeekCtx->representationInfo->parentObj->hasAudio;
    mediaFileSegmentInfo->hasVideo = representationSeekCtx->representationInfo->parentObj->hasVideo;

    mediaFileSegmentInfo->absoluteUri = B_PlaybackIp_MpegDashCreateAbsoluteUrlForSeek(playback_ip, representationSeekCtx, urlKind);

    if (!mediaFileSegmentInfo->absoluteUri) {
        PRINTMSG_SEEK(("%s: Could not build URI for current seek context", __FUNCTION__));
        goto error;
    }

    if ( mpegDash_parse_absolute_url(     &mediaFileSegmentInfo->protocol,
                                 &mediaFileSegmentInfo->server,
                                 &mediaFileSegmentInfo->port,
                                 &mediaFileSegmentInfo->uri,
                                 mediaFileSegmentInfo->absoluteUri) == false )
    {
        BDBG_ERR(("%s: Failed to parse URI", __FUNCTION__));
        goto error;
    }

    if ( mediaFileSegmentInfo->protocol != B_PlaybackIpProtocol_eHttp &&
         mediaFileSegmentInfo->protocol != B_PlaybackIpProtocol_eHttps )
    {
        BDBG_ERR(("%s:%d URI is for non-HTTP(S) protocol", __FUNCTION__, __LINE__));
        goto error;
    }

    PRINTMSG_SEEK(("%s:%d : Built mediaFileSegmentInfo: uri: %s ", __FUNCTION__, __LINE__, mediaFileSegmentInfo->absoluteUri   ));
    return mediaFileSegmentInfo;

error:
    if (mediaFileSegmentInfo)  B_PlaybackIp_MpegDashFreeMediaFileSegmentInfo(mediaFileSegmentInfo);
    return NULL;
}


/*****************************************************************************
 * The following is a "bfile" abstraction for writing to a Segment Buffer.
 * Note that this "bfile_io_random_write" is an extension to a normal
 * "bfile_io_write" which adds seek() and bounds() methods.
 *****************************************************************************/
typedef struct bfile_io_random_write *bfile_io_random_write_t;

struct bfile_io_random_write {  /* Same as bfile_io_write, but added seek() and bounds() functions. */
    ssize_t (*write)(bfile_io_random_write_t fd, const void *buf, size_t length); /* this method is used to write data, it returns number of bytes written */
    off_t (*trim)(bfile_io_random_write_t fd, off_t trim_pos); /* this method is used to truncate file */
    off_t (*seek)(bfile_io_random_write_t fd, off_t offset, int whence);  /* this method is used to move next write location */
    int (*bounds)(bfile_io_random_write_t fd, off_t *first, off_t *last); /* this method is used to return size of the file */
    bfile_io_priority priority; /* this member is used to request current priority of I/O transaction */
};

BDBG_OBJECT_ID(bfile_io_random_write_segment_buffer);
struct bfile_io_random_write_segment_buffer {
    struct bfile_io_random_write ops;
    MpegDashSegmentBuffer  * segmentBuffer;
    off_t pos;
    BDBG_OBJECT(bfile_io_random_write_segment_buffer)
};

static ssize_t
b_segment_buffer_random_write_write(bfile_io_random_write_t fd, const void *buf, size_t length)
{
    struct bfile_io_random_write_segment_buffer *f=(void *)fd;
    char * pDst;
    size_t myLength;

    BDBG_OBJECT_ASSERT(f, bfile_io_random_write_segment_buffer);
    PRINTMSG_SEGBUF_WRITE(("%s:%d:  %#lx %lu:%u", __FUNCTION__, __LINE__, (unsigned long)fd, (unsigned long)(f->pos), (unsigned)length));

    if(!f->segmentBuffer)  return -1;

    pDst = f->segmentBuffer->buffer;

    if (f->pos >= f->segmentBuffer->bufferSize) {   /* Trying to write past end of segment buffer. */
        return 0;
    }

    pDst += f->pos;     /* pDst -> write position. */

    myLength = f->segmentBuffer->bufferSize - f->pos;   /* Maximum allowed to write. */

    if (length < myLength) {
        myLength = length;      /* Reduce write count from max to that requested. */
    }

    BKNI_Memcpy(pDst, buf, myLength);

    f->pos += myLength;
    if (f->pos > f->segmentBuffer->bufferDepth) {
        f->segmentBuffer->bufferDepth = f->pos;
    }

    return myLength;
}

static off_t
b_segment_buffer_random_write_trim(bfile_io_random_write_t fd, off_t trim_pos)
{
    struct bfile_io_random_write_segment_buffer *f=(void *)fd;

    BDBG_OBJECT_ASSERT(f, bfile_io_random_write_segment_buffer);
    PRINTMSG_SEGBUF_WRITE(("bfile_io_random_write_segment_buffer_trim: %#lx %lu:%u", (unsigned long)fd, (unsigned long)(f->pos), (unsigned)trim_pos));

    if(!f->segmentBuffer)  return -1;

    if (trim_pos < f->segmentBuffer->bufferDepth) {
        f->segmentBuffer->bufferDepth = trim_pos;
    }

    return f->segmentBuffer->bufferDepth;
}

static off_t
b_segment_buffer_random_write_seek(bfile_io_random_write_t fd, off_t offset, int whence)
{
    struct bfile_io_random_write_segment_buffer *f=(void *)fd;
    BDBG_OBJECT_ASSERT(f, bfile_io_random_write_segment_buffer);

    PRINTMSG_SEGBUF_WRITE(("b_segment_buffer_random_write_seek: %#lx %lu:%d", (unsigned long)fd, (unsigned long)offset, whence));

    if(f->segmentBuffer) {
        off_t first, last;

        first = 0;
        last = f->segmentBuffer->bufferDepth;

        if(whence==SEEK_END) {
            offset = last + offset;
        } else if (whence==SEEK_CUR) {
            offset = f->pos + offset;
        }

        if(offset<first) {
            offset = first;
        } else if (offset>last) {
            offset = last;
        }
        f->pos = offset;
    }

    return offset;
}

static int
b_segment_buffer_random_write_bounds(bfile_io_random_write_t fd, off_t *first, off_t *last)
{
    struct bfile_io_random_write_segment_buffer *f=(void *)fd;
    BDBG_OBJECT_ASSERT(f, bfile_io_random_write_segment_buffer);

    *first = *last = 0;

    if (f->segmentBuffer) {
        *last = f->segmentBuffer->bufferDepth;
    }
    return 0;
}

static const struct bfile_io_random_write b_segment_buffer_write_ops = {
    b_segment_buffer_random_write_write,
    b_segment_buffer_random_write_trim,
    b_segment_buffer_random_write_seek,
    b_segment_buffer_random_write_bounds,
    BIO_DEFAULT_PRIORITY
};

bfile_io_random_write_t
bfile_segment_buffer_random_write_attach(MpegDashSegmentBuffer  * segmentBuffer)
{
    struct bfile_io_random_write_segment_buffer *f;
    f=BKNI_Malloc(sizeof(*f));
    if (!f) { return NULL; }
    BDBG_OBJECT_INIT(f, bfile_io_random_write_segment_buffer );
    f->ops = b_segment_buffer_write_ops;
    f->segmentBuffer = segmentBuffer;
    f->pos = segmentBuffer->bufferDepth;
    return &f->ops;
}

void
bfile_segment_buffer_random_write_detach(bfile_io_random_write_t fd)
{
    struct bfile_io_random_write_segment_buffer *f=(void *)fd;
    BDBG_OBJECT_ASSERT(f, bfile_io_random_write_segment_buffer );

    /* Flush any data remaining in the segment_buffer. */

    BDBG_OBJECT_DESTROY(f, bfile_io_random_write_segment_buffer);
    BKNI_Free(f);

    return;
}


/*****************************************************************************
 * The following is a "bfile" abstraction for reading from a Segment Buffer.
 *****************************************************************************/
BDBG_OBJECT_ID(bfile_io_read_segment_buffer);

struct bfile_io_read_segment_buffer {
    struct bfile_io_read ops; /* shall be the first member */
    MpegDashSegmentBuffer  * segmentBuffer;
    off_t pos;
    BDBG_OBJECT(bfile_io_read_segment_buffer)
};

static ssize_t
b_segment_buffer_read(bfile_io_read_t fd, void *buf, size_t length)
{
    struct bfile_io_read_segment_buffer *f=(void *)fd;
    char * pSrc;
    size_t myLength;

    BDBG_OBJECT_ASSERT(f, bfile_io_read_segment_buffer);
    PRINTMSG_SEGBUF_READ(("b_segment_buffer_read: %#lx %lu:%u", (unsigned long)fd, (unsigned long)(f->pos), (unsigned)length));

    pSrc = f->segmentBuffer->buffer;

    if (f->pos >= f->segmentBuffer->bufferDepth) {
        return 0;
    }

    pSrc += f->pos;

    myLength = f->segmentBuffer->bufferDepth - f->pos;

    if (length < myLength) {
        myLength = length;
    }

    BKNI_Memcpy(buf, pSrc, myLength );

    return myLength;
}

static off_t
b_segment_buffer_seek(bfile_io_read_t fd, off_t offset, int whence)
{
    struct bfile_io_read_segment_buffer *f=(void *)fd;
    BDBG_OBJECT_ASSERT(f, bfile_io_read_segment_buffer);

    PRINTMSG_SEGBUF_READ(("b_segment_buffer_seek: %#lx %lu:%d", (unsigned long)fd, (unsigned long)offset, whence));

    if(f->segmentBuffer) {
        off_t first, last;

        switch(whence) {
        case SEEK_CUR:
        case SEEK_END:
            first = 0;
            last = f->segmentBuffer->bufferDepth;

            if(whence==SEEK_END) {
                offset = last + offset;
            } else {
                offset = f->pos + offset;
            }
            if(offset<first) {
                offset = first;
            } else if (offset>last) {
                offset = last;
            }
            /* keep going */
        case SEEK_SET:
        default:
            f->pos = offset;
            break;
        }
        return offset;
    }

    return -1;
}

static int
b_segment_buffer_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
    struct bfile_io_read_segment_buffer *f=(void *)fd;
    BDBG_OBJECT_ASSERT(f, bfile_io_read_segment_buffer);

    *first = *last = 0;

    if (f->segmentBuffer) {
        *last = f->segmentBuffer->bufferDepth;
    }
    return 0;
}

static const struct bfile_io_read b_segment_buffer_read_ops = {
    b_segment_buffer_read,
    b_segment_buffer_seek,
    b_segment_buffer_bounds,
    BIO_DEFAULT_PRIORITY
};

bfile_io_read_t
bfile_segment_buffer_read_attach(MpegDashSegmentBuffer  * segmentBuffer)
{
    struct bfile_io_read_segment_buffer *f;
    f=BKNI_Malloc(sizeof(*f));
    if (!f) { return NULL; }
    BDBG_OBJECT_INIT(f, bfile_io_read_segment_buffer);
    f->ops = b_segment_buffer_read_ops;
    f->segmentBuffer = segmentBuffer;
    f->pos = 0;
    return &f->ops;
}

void
bfile_segment_buffer_read_detach(bfile_io_read_t file)
{
    struct bfile_io_read_segment_buffer *f=(void *)file;
    BDBG_OBJECT_ASSERT(f, bfile_io_read_segment_buffer);

    BDBG_OBJECT_DESTROY(f, bfile_io_read_segment_buffer);
    BKNI_Free(f);

    return;
}


/*****************************************************************************
 * Create a batom that's mapped to a range in a segment buffer.
 * This allows us to do more detailed probing of a box who's
 * location is returned by the bmedia probe logic.
 *****************************************************************************/
static batom_t
B_PlaybackIp_MpegDashSegmentBufferCreateAtom(B_PlaybackIpHandle playback_ip, MpegDashSegmentBuffer  * segmentBuffer, uint64_t offset, uint64_t size)
{
    MpegDashSessionState  * mpegDashSession  = playback_ip->mpegDashSessionState;
    char                  * myFuncName = "SegBufCreateAtom";
    batom_t                 atom;

    BSTD_UNUSED(playback_ip);
    BDBG_ASSERT(segmentBuffer);
    BDBG_ASSERT(playback_ip);

    /* Get the range's offset and size, then make sure it's within
     * the segment buffer. */
    if (offset + size > (unsigned)segmentBuffer->bufferDepth)
    {
       BDBG_ERR(("%s:%d: Box is beyond end of segment!  Segment length:%u  Box start:"BDBG_UINT64_FMT" size:"BDBG_UINT64_FMT,
                 myFuncName, __LINE__, (unsigned)segmentBuffer->bufferDepth, BDBG_UINT64_ARG(offset), BDBG_UINT64_ARG(size) ));
       return (NULL);
    }

    /* Now make range into an atom. */
    atom = batom_from_range(mpegDashSession->factory, segmentBuffer->buffer + offset, size, NULL, NULL);

    if (!atom )
    {
        BDBG_ERR(("%s:%d: Failed to create batom from segmentBuffer!  Start addr:%p size:"BDBG_UINT64_FMT,
                  myFuncName, __LINE__, (void *)(segmentBuffer->buffer + offset), BDBG_UINT64_ARG(size)));
    }
    return atom;
}


/*****************************************************************************
 * Locate a sub-box of a specified type inside of a specified
 * parent box.
 *****************************************************************************/
static batom_t
B_PlaybackIp_MpegDashCreateSubboxAtom(B_PlaybackIpHandle playback_ip, batom_t parentAtom, uint32_t subboxType)
{
    batom_t         atom = NULL;
    batom_cursor    cursor;

    BSTD_UNUSED(playback_ip);
    BDBG_ASSERT(playback_ip);

    batom_cursor_from_atom(&cursor, parentAtom);

    PRINTMSG_PROBE(("%s:%d : Looking for "B_MP4_TYPE_FORMAT" box...", __FUNCTION__, __LINE__ , B_MP4_TYPE_ARG(subboxType)));
    for(;;)
    {
        bmp4_box        box;
        size_t          box_hdr_size = bmp4_parse_box(&cursor, &box);
        batom_cursor    cursor_end;

        if(box_hdr_size==0) {
            break;   /* At end of cursor, give up. */
        }

        if(box.type == subboxType) {
            PRINTMSG_PROBE(("%s:%d : Found "B_MP4_TYPE_FORMAT" box! Size: %"PRIu64, __FUNCTION__, __LINE__ , B_MP4_TYPE_ARG(box.type), box.size - box_hdr_size));

            batom_cursor_clone(&cursor_end, &cursor);
            batom_cursor_skip(&cursor_end, box.size - box_hdr_size);
            atom = batom_extract(parentAtom, &cursor, &cursor_end, NULL, NULL);
            break;
        }

        /* Not the box we want, skip it and look at next one. */
        if(box.size>box_hdr_size) {
            size_t  skipcount = box.size - box_hdr_size;
            PRINTMSG_PROBE(("%s:%d : Found "B_MP4_TYPE_FORMAT" box. Size: %"PRIu64", Skipping...", __FUNCTION__, __LINE__ , B_MP4_TYPE_ARG(box.type), box.size - box_hdr_size));

            if (skipcount != batom_cursor_skip(&cursor, skipcount) ) {
                BDBG_WRN(("%s:%d : Can't find "B_MP4_TYPE_FORMAT" box", __FUNCTION__, __LINE__,B_MP4_TYPE_ARG(subboxType) ));
                break;
            }
        }
    }

    return atom;
}


/*****************************************************************************
 *  Parse the Movie Header box for the mp4 initialization
 *  segment that's in the specified segment buffer.
 *****************************************************************************/
bool
B_PlaybackIp_MpegDashParseMvhdForStream(B_PlaybackIpHandle playback_ip, MpegDashSegmentBuffer *segmentBuffer, const bmp4_probe_stream *bmp4_stream, bmp4_movieheaderbox *movieheader)
{
    batom_t atom_moov = NULL;
    batom_t atom_mvhd = NULL;

    BSTD_UNUSED(playback_ip);

    BDBG_ASSERT(playback_ip);
    BDBG_ASSERT(segmentBuffer);
    BDBG_ASSERT(bmp4_stream);
    BDBG_ASSERT(movieheader);

    /* Create a batom from the "moov" box that bmedia_probe found
     * in the segment buffer.*/
    atom_moov = B_PlaybackIp_MpegDashSegmentBufferCreateAtom(playback_ip, segmentBuffer, bmp4_stream->moov.offset, bmp4_stream->moov.size);
    if (!atom_moov) { goto error; }

    /* Create a batom from the "mvhd" subbox inside the "moov"
     * box. */
    atom_mvhd = B_PlaybackIp_MpegDashCreateSubboxAtom(playback_ip, atom_moov, BMP4_TYPE('m','v','h','d'));

    if (!atom_mvhd)
    {
        BDBG_ERR(("%s: Can't find Movie Header (mvhd) box in Movie (moov) box!", __FUNCTION__));
        batom_dump(atom_moov, "moov box");
        goto error;
    }

    /* Parse the Movie Header box into the caller's buffer. */
    if (!bmp4_parse_movieheader(atom_mvhd, movieheader))
    {
        BDBG_ERR(("%s: Can't parse Movie Header (mvhd) box!", __FUNCTION__));
        batom_dump(atom_mvhd, "mvhd box");
        goto error;
    }

    batom_release(atom_moov);
    batom_release(atom_mvhd);

    return true;

error:
    if (atom_moov) {batom_release(atom_moov);}
    if (atom_mvhd) {batom_release(atom_mvhd);}

    return false;
}

/*****************************************************************************
 *  Parse the Media Header box for the specified track from mp4
 *  initialization segment that's in the specified segment
 *  buffer.
 *****************************************************************************/
bool
B_PlaybackIp_MpegDashParseMdhdForTrackid(B_PlaybackIpHandle playback_ip, MpegDashSegmentBuffer  * segmentBuffer, const bmp4_probe_track * bmp4_track, unsigned trackId, bmp4_mediaheaderbox *mediaheader)
{
    batom_t atom_mdhd = NULL;

    BSTD_UNUSED(playback_ip);

    BDBG_ASSERT(playback_ip);
    BDBG_ASSERT(segmentBuffer);
    BDBG_ASSERT(bmp4_track);
    BDBG_ASSERT(mediaheader);

    /* Create a batom from the Movie Extends (mvex) box in the
     * segment buffer.*/
    atom_mdhd = B_PlaybackIp_MpegDashSegmentBufferCreateAtom(playback_ip, segmentBuffer, bmp4_track->mediaheader.offset, bmp4_track->mediaheader.size);
    if (!atom_mdhd) { goto error; }

    /* Parse the Media Header box into the caller's buffer. */
    if (!bmp4_parse_mediaheader(atom_mdhd, mediaheader))
    {
        BDBG_ERR(("%s: Can't parse Media Header (mdhd) box for trackID: %u!", __FUNCTION__, trackId));
        batom_dump(atom_mdhd, "mdhd box");
        goto error;
    }

    batom_release(atom_mdhd);
    return true;

error:
    if (atom_mdhd) {batom_release(atom_mdhd);}
    return false;
}


/*****************************************************************************
 *  Parse the Track Extends box for the specified
 *  trackID from mp4 initialization segment that's in the
 *  specified segment buffer.
 *****************************************************************************/
bool
B_PlaybackIp_MpegDashParseTrexForTrackid(B_PlaybackIpHandle playback_ip, MpegDashSegmentBuffer  * segmentBuffer, const bmp4_probe_stream * bmp4_stream, unsigned track_ID, bmp4_trackextendsbox *trackextends)
{
    batom_t         atom_mvex = NULL;
    batom_t         atom_trex = NULL;
    batom_cursor    cursor;

    BDBG_ASSERT(playback_ip);
    BDBG_ASSERT(segmentBuffer);
    BDBG_ASSERT(bmp4_stream);
    BDBG_ASSERT(trackextends);

    /* Create a batom from the Movie Extends (mvex) box in the
     * segment buffer. */
    atom_mvex = B_PlaybackIp_MpegDashSegmentBufferCreateAtom(playback_ip, segmentBuffer, bmp4_stream->mvex.offset, bmp4_stream->mvex.size);
    if (!atom_mvex) { goto error; }

    /* Look through the sub-boxes until we find the TrackExtends
     * box for the Track_ID that we're interested in. */
    batom_cursor_from_atom(&cursor, atom_mvex);

    PRINTMSG_PROBE(("%s:%d : Looking for Track Extends (trex) box...", __FUNCTION__, __LINE__));
    for(;;)
    {
        bmp4_box                box;
        size_t                  box_hdr_size = bmp4_parse_box(&cursor, &box);
        batom_cursor            cursor_end;
        bmp4_trackextendsbox    my_track_extends;

        if(box_hdr_size==0) {
            goto error;   /* At end of cursor, give up. */
        }

        if(box.type == BMP4_TYPE('t','r','e','x'))
        {
            PRINTMSG_PROBE(("%s:%d : Found a "B_MP4_TYPE_FORMAT" box! Size: %"PRIu64, __FUNCTION__, __LINE__ , B_MP4_TYPE_ARG(box.type), box.size - box_hdr_size));

            /* We have the trex box.  Now make it into a batom so we can
             * parse it. */
            batom_cursor_clone(&cursor_end, &cursor);
            batom_cursor_skip(&cursor_end, box.size - box_hdr_size);    /* skip to end of trex box */
            atom_trex = batom_extract(atom_mvex, &cursor, &cursor_end, NULL, NULL);

            if (!bmp4_parse_trackextends(atom_trex, &my_track_extends))
            {
                /* We can't parse this trex box, but don't give up... it might
                 * not be the one that we're looking for, so keep looking. */
                BDBG_ERR(("%s: Can't parse Track Extends (trex) box!", __FUNCTION__));
                batom_dump(atom_trex, "trex box");
            }
            else
            {
                /* If this trex is for the specified track_ID, then just copy
                 * it to the callers buffer and we're done.  Otherwise, keep
                 * looking at any other trex boxes. */
                if (my_track_extends.track_ID == track_ID)
                {
                    PRINTMSG_PROBE(("%s:%d : Found the Track Extends (trex) box for track ID: %u! Size: %"PRIu64, __FUNCTION__, __LINE__ , track_ID, box.size - box_hdr_size  ));
                    BKNI_Memcpy(trackextends, &my_track_extends, sizeof *trackextends);
                    break;
                }
            }
        }

        if(box.size>box_hdr_size) {
            size_t  skipcount = box.size - box_hdr_size;
            PRINTMSG_PROBE(("%s:%d : Found "B_MP4_TYPE_FORMAT" box. Size: %"PRIu64", Skipping...", __FUNCTION__, __LINE__ , B_MP4_TYPE_ARG(box.type), box.size - box_hdr_size));

            if (skipcount != batom_cursor_skip(&cursor, skipcount) ) {
                goto error;
            }
        }
    }
    if (atom_mvex) {batom_release(atom_mvex);}
    if (atom_trex) {batom_release(atom_trex);}
    return true;

error:
    BDBG_WRN(("%s:%d : Can't find Track Extends (trex) box for track_ID:%u", __FUNCTION__, __LINE__, track_ID));
    if (atom_mvex) {batom_release(atom_mvex);}
    if (atom_trex) {batom_release(atom_trex);}
    return false;
}

/*****************************************************************************
 *  Extract the Decoder Specific Data for the specified trackID
 *  from mp4 initialization segment that's in the specified
 *  segment buffer.
 *****************************************************************************/
bool
B_PlaybackIp_MpegDashGetDecoderSpecificInfoForTrackid(
                            B_PlaybackIpHandle          playback_ip,
                            MpegDashSegmentBuffer     * segmentBuffer,
                            const bmp4_probe_track    * bmp4_track,
                            unsigned                    trackId,
                            size_t                    * pDecSpecInfoLen,
                            void                     ** pDecSpecInfoAddr,
                            uint32_t                  * pFourcc)
{
    batom_t             atom_stsd = NULL;
    bmp4_sample_info    sample_info;
    bool                sample_info_valid = false;
    bmp4_sampleentry  * sampleentry;
    batom_cursor        cursor;
    bmp4_box            box;
    size_t              box_hdr_size;
    uint32_t            handler_type;
    size_t              decSpecInfoLen = 0;
    const void        * decSpecInfoAddr = NULL;

    BDBG_ASSERT(playback_ip);
    BDBG_ASSERT(segmentBuffer);
    BDBG_ASSERT(bmp4_track);
    BDBG_ASSERT(pDecSpecInfoLen);
    BDBG_ASSERT(pDecSpecInfoAddr);

    /* Create a batom from the Sample Description (stsd) box in
     * the segment buffer.*/
    atom_stsd = B_PlaybackIp_MpegDashSegmentBufferCreateAtom(playback_ip, segmentBuffer, bmp4_track->sampledescription.offset, bmp4_track->sampledescription.size);
    if (!atom_stsd) { goto error; }

    /* Choose a handler type based on audio or video. */
    switch (bmp4_track->media.type)
    {
    case bmedia_track_type_video:
        handler_type = BMP4_HANDLER_VISUAL;
        break;
    case bmedia_track_type_audio:
        handler_type = BMP4_HANDLER_AUDIO;
        break;
    default:
        BDBG_ERR(("%s: Track is not audio or video!", __FUNCTION__));
        goto error;
    }

    /* Parse the Sample Description and it's SampleTable
     * sub-boxes into a bmp4_sampleentry structure. */
    if (!bmp4_parse_sample_info(atom_stsd, &sample_info, handler_type))
    {
        BDBG_ERR(("%s: Can't parse Sample Description (stsd) box!", __FUNCTION__));
        batom_dump(atom_stsd, "stsd box");
        goto error;
    }
    sample_info_valid = true;
    sampleentry = sample_info.entries[0];

    PRINTMSG_PROBE(("%s:%d : Getting Decoder Specific Data for Sample Entry: "B_MP4_TYPE_FORMAT, __FUNCTION__, __LINE__, B_MP4_TYPE_ARG(sampleentry->type) ));

    /* Extract the Decoder Specific Info according to the sample
     * type. */
    switch (sampleentry->sample_type)
    {
        /* AVC and HEVC entries can be handled the same way... the bmp4_sampleentry struct contains a
         *  "codecprivate.offset" field that specifies the offset from the start of the stsd atom to
         *  either:
         *       for AVC:   the "avcC" box that contains the AVCDecoderConfigurationRecord.
         *       for HEVC:  the "hvcC" box that contains the HEVCDecoderConfigurationRecord. */
        case bmp4_sample_type_avc:
        case bmp4_sample_type_hevc:
        {
            PRINTMSG_PROBE(("%s:%d: SampleType: AVC/HEVC", __FUNCTION__, __LINE__));

            /* Use a cursor to find the start of the avcC or hvcC box. */
            batom_cursor_from_atom(&cursor, atom_stsd);

            batom_cursor_skip(&cursor, sampleentry->codecprivate.offset); /* move to start of box. */

            box_hdr_size = bmp4_parse_box(&cursor, &box);                            /* skip over box header. */

            /* Get the size of the avcC/hvcC data inside the box (without the box header). */
            decSpecInfoLen = box.size - box_hdr_size;

            /* Get a pointer to the avcC/hvcC data (after the box header). */
            decSpecInfoAddr = BATOM_CONTINUOUS(&cursor, decSpecInfoLen );
            if (!decSpecInfoAddr)
            {
                BDBG_ERR(("%s: Can't get address of Decoder Specific Info for track %u!", __FUNCTION__, trackId));
                goto error;
            }
            break;
        }

        /* For mp4a AAC audio, the Decoder Specific Info has
         *  already been saved by the bmp4_parse_sample_info() function. */
        case bmp4_sample_type_mp4a:
        {
            bmpeg4_es_descriptor  *descriptor = &sampleentry->codec.mp4a.mpeg4;

            PRINTMSG_PROBE(("%s:%d: SampleType: mp4a", __FUNCTION__, __LINE__));

            if (descriptor->objectTypeIndication == BMPEG4_Audio_ISO_IEC_14496_3 ||
                descriptor->objectTypeIndication == BMPEG4_Audio_ISO_IEC_13818_7)
            {
                decSpecInfoLen = descriptor->decoder.iso_14496_3.aac_info_size;
                decSpecInfoAddr = descriptor->decoder.iso_14496_3.aac_info;
            }
            break;
        } /* case bmp4_sample_type_mp4a: */
        default:
        {
            break;
        }
    } /* switch (sampleentry->sample_type) */


    /* If already allocated, but not proper size, free it. */
    if (*pDecSpecInfoLen > 0 && *pDecSpecInfoLen != decSpecInfoLen) {

        BKNI_Free(*pDecSpecInfoAddr);
        *pDecSpecInfoAddr = NULL;
        *pDecSpecInfoLen = 0;
    }

    /* If not already allocated, malloc space for codec specific data. */
    if (decSpecInfoLen > 0 && *pDecSpecInfoLen == 0) {
        *pDecSpecInfoAddr = BKNI_Malloc(decSpecInfoLen);
        if (!*pDecSpecInfoAddr) {
            BDBG_ERR(("%s: Failed to allocate %d bytes of memory for Decoder Specific Info", __FUNCTION__, (int)decSpecInfoLen));
            goto error;
        }
        *pDecSpecInfoLen = decSpecInfoLen;
    }

    if (decSpecInfoLen > 0) {
        BKNI_Memcpy(*pDecSpecInfoAddr, decSpecInfoAddr, decSpecInfoLen);
    }
    PRINTMSG_PROBE_DUMP("Decoder Specific Info ", "decSpecInfoAddr", decSpecInfoAddr, decSpecInfoLen);

    /* Determine the fourcc code (see www.fourcc.org) for the
     * codec that we'll need to decode this content.  For the
     * media types that we currently support, the fourcc code is
     * just an endian-reversed version of the SampleEntry type. So
     * we'll just go with that for now. */
    *pFourcc = ((sampleentry->type >> 24) & 0x000000ff) |
               ((sampleentry->type >>  8) & 0x0000ff00) |
               ((sampleentry->type <<  8) & 0x00ff0000) |
               ((sampleentry->type << 24) & 0xff000000);

    PRINTMSG_PROBE(("%s:%d : Sample Type: "B_MP4_TYPE_FORMAT" fourcc: "BMEDIA_FOURCC_FORMAT, __FUNCTION__, __LINE__, B_MP4_TYPE_ARG(sampleentry->type), BMEDIA_FOURCC_ARG(*pFourcc)));


    if (sample_info_valid){bmp4_free_sample_info(&sample_info);}
    if (atom_stsd) {batom_release(atom_stsd);}
    return true;

error:
    if (sample_info_valid){bmp4_free_sample_info(&sample_info);}
    if (atom_stsd) {batom_release(atom_stsd);}
    return false;
}


/*****************************************************************************
 * For a given RepresentationSeekContext, determine the media characteristics
 * (PSI) by downloading the initialization segment (or perhaps a media segment),
 * then probing for the various media attributes.
 *****************************************************************************/
bool
B_PlaybackIp_MpegDashProbeForRepresentationStreamInfo(B_PlaybackIpHandle playback_ip, MpegDashRepresentationSeekContext  * representationSeekCtx, MpegDashSegmentBuffer  * segmentBuffer)
{
    MpegDashSessionState        * mpegDashSession  = playback_ip->mpegDashSessionState;
    MpegDashRepresentationInfo  * representationInfo;
    MediaFileSegmentInfo        * mediaFileSegmentInfo = NULL;
    MpegDashDownloadContext     * downloadContext = &mpegDashSession->downloadContext;
    bool                          serverClosed     = false;
    char                        * myFuncName = "ProbeForReprStreamInfo";
    bfile_io_read_t               fd2         = NULL;
    bmedia_probe_t                probe       = NULL;
    const bmedia_probe_stream   * stream      = NULL;
    bmedia_probe_track          * track       = NULL;
    const bmp4_probe_stream     * bmp4_stream = NULL;
    bmp4_probe_track            * bmp4_track  = NULL;
    B_PlaybackIpPsiInfo         * psi         = &playback_ip->psi;
    bool                          returnCode = false;    /* assume failure */


    BSTD_UNUSED(playback_ip);

    BDBG_ASSERT(representationSeekCtx);

    representationInfo = representationSeekCtx->representationInfo;
    BDBG_ASSERT(representationInfo);

    representationInfo->probeInfoIsValid = false;   /* Assume that something will go wrong. */

    /* Create a MediaFileSegmentInfo struct to download the init segment for the seek context. */
    mediaFileSegmentInfo = B_PlaybackIp_MpegDashCreateMediaFileSegmentInfoFromSeek(playback_ip, representationSeekCtx, MPEG_DASH_URL_KIND_INITIALIZATION);
    if (!mediaFileSegmentInfo) {
        PRINTMSG_PROBE(("%s: Can't find initialization segment for representation %s", myFuncName, representationInfo->id));

        /* If there's no initialization segment, try probing the
         *  media segment. */
        mediaFileSegmentInfo = B_PlaybackIp_MpegDashCreateMediaFileSegmentInfoFromSeek(playback_ip, representationSeekCtx, MPEG_DASH_URL_KIND_MEDIA);
        if (!mediaFileSegmentInfo) {
            BDBG_WRN(("%s: Can't find media segment for representation %s", myFuncName, representationInfo->id));
            BDBG_ERR(("%s: Unable to probe for stream info for representation %s", myFuncName, representationInfo->id));
            goto error;
        }
    }

    /* Set up a session and start the download. */
    PRINTMSG_DOWNLOAD(("%s:%d: Started  download: %s", myFuncName, __LINE__, mediaFileSegmentInfo->uri));
    PRINTMSG_SUMMARY(( "%s:%d: Started  download: %s", myFuncName, __LINE__, mediaFileSegmentInfo->uri));
    if (B_PlaybackIp_MpegDashMediaSegmentDownloadStart(
                    playback_ip,
                    downloadContext,
                    mediaFileSegmentInfo,
                    &playback_ip->socketState.fd,
                    segmentBuffer) == false) {
        BDBG_ERR(("%s: ERROR: Socket setup or HTTP request/response failed for downloading Media Segment", myFuncName));
        goto error;
    }

    /* Now finish the download */
    if (B_PlaybackIp_MpegDashMediaSegmentDownloadFinish(playback_ip, downloadContext,  &serverClosed) != true) {
        BDBG_ERR(("%s: failed to download the current media segment", myFuncName));
        goto error;
    }
    /* GARYWASHERE: Need to check serverClosed flag. */

    PRINTMSG_DOWNLOAD(("%s:%d: Finished download: %s", myFuncName, __LINE__, mediaFileSegmentInfo->uri));
    PRINTMSG_SUMMARY(( "%s:%d: Finished download: %s", myFuncName, __LINE__, mediaFileSegmentInfo->uri));

    B_PlaybackIp_MpegDashAddBandwidthSample(&mpegDashSession->bandwidthContext, downloadContext->elapsedTimeInMs, downloadContext->totalBytesReadSoFar);

    /* Do the parse/probe of the downloaded segment. */
    fd2 = bfile_segment_buffer_read_attach(segmentBuffer);
    BDBG_ASSERT(fd2);

    probe = bmedia_probe_create();
    {
        bmedia_probe_config     probe_config;

        bmedia_probe_default_cfg(&probe_config);
        probe_config.type = bstream_mpeg_type_mp4;
        stream = bmedia_probe_parse(probe, fd2, &probe_config);
        if (!stream) {
            probe_config.type = bstream_mpeg_type_ts;
            stream = bmedia_probe_parse(probe, fd2, &probe_config);
        }
    }
    if(!stream ) {
        BDBG_ERR(("%s: No stream returned from probe of initialization segment", myFuncName));
        goto error;
    }

#if MPEG_DASH_UNIT_TEST  /* ==================== Start of Unit Test Code ==================== */
    {
        char  print_buf[1024];

        bmedia_stream_to_string(stream, print_buf, sizeof print_buf);
        PRINTMSG_PROBE(("%s:%d: Stream Info: %s", myFuncName, __LINE__, print_buf));
    }
#endif /* MPEG_DASH_UNIT_TEST  ==================== End of Unit Test Code ==================== */

    if (BLST_SQ_FIRST(&stream->tracks) == NULL) {
        BDBG_ERR(("%s: No tracks returned from probe of initialization segment", myFuncName));
        goto error;
    }

    /* Make sure the stream is mp4 (ISOBMFF), then get the pointer
     * to the mp4-specific stream data. */
    if (stream->type == bstream_mpeg_type_mp4)
    {
        bmp4_stream = (bmp4_probe_stream *) stream;
    }
    else if (stream->type != bstream_mpeg_type_ts)
    {
        BDBG_ERR(("%s: Probe detected unexpected stream type (bstream_mpeg_type): %d", myFuncName, stream->type));
        goto error;
    }

    psi->mpegType = B_PlaybackIp_UtilsMpegtype2ToNexus(stream->type);
    if (psi->mpegType == NEXUS_TransportType_eMp4 && bmp4_stream && bmp4_stream->mvex.size)
    {
        psi->mpegType = NEXUS_TransportType_eMp4Fragment;
    }
    mpegDashSession->mpegType = psi->mpegType;

    PRINTMSG_PROBE(("%s:%d: stream->type=0x%x", myFuncName, __LINE__, stream->type));
    {
        const char  * mpdType = representationInfo->parentObj->parentObj->parentObj->type;
        if (strcmp(mpdType, "dynamic")) {
            /* unbounded case, i.e. live stream, so set duration to large value */
            /* media browser needs the duration to know when to stop the playback */
            psi->duration = 0;
        }
        else
        {
            /* GARYWASHERE: Need to figure out the duration.  Maybe Mpd.mediaPresentationDuration?? */
        }
    }

    psi->mpegDashSessionEnabled = true;
    psi->transportTimeStampEnabled = false;  /* Only applies to TS streams. */

    if (bmp4_stream)
    {
        bmp4_movieheaderbox     movieheader;
        B_PlaybackIp_MpegDashParseMvhdForStream(playback_ip, segmentBuffer, bmp4_stream, &movieheader);

        if (movieheader.duration)
            psi->duration = (movieheader.duration * 1000) / movieheader.timescale;

        PRINTMSG_PROBE(("%s:%d: movieheader.timescale=%u", myFuncName, __LINE__, movieheader.timescale));
        PRINTMSG_PROBE(("%s:%d: movieheader.duration="BDBG_UINT64_FMT, myFuncName, __LINE__, BDBG_UINT64_ARG(movieheader.duration)));
        PRINTMSG_PROBE(("%s:%d: psi->duration=%u", myFuncName, __LINE__, psi->duration));
    }

    psi->avgBitRate = stream->max_bitrate;
    PRINTMSG_PROBE(("%s:%d: stream->max_bitrate=%u", myFuncName, __LINE__, stream->max_bitrate));

    if (psi->avgBitRate) {
        /* GARYWASHERE: Use segment buffer size to determine maxBufferDuration. */
        psi->maxBufferDuration = 1000; /* (playback_ip->dataCache[0].size*8 / psi->avgBitRate)*1000;   in msec */
    } else {
        PRINTMSG_PROBE(("%s: Warning: Media Probe couldn't determine the avg bitrate of stream!!!", myFuncName));
        psi->avgBitRate = 0;
    }

    PRINTMSG_PROBE(("Media Details: container type %d, index %d, avg bitrate %d, duration %d, cache buffer in msec %lu, # of programs %d, # of tracks (streams) %d",
                psi->mpegType, stream->index, psi->avgBitRate, psi->duration, psi->maxBufferDuration, stream->nprograms, stream->ntracks));

    for(track=BLST_SQ_FIRST(&stream->tracks) ; track ; track=BLST_SQ_NEXT(track, link)) {
        bool foundAudio = false;
        bool foundVideo = false;

        /* Get the pointer to the mp4-specific track data.*/
        bmp4_track = (bmp4_probe_track *)track;

        PRINTMSG_PROBE(("%s:%d: Found track->number=%u", myFuncName, __LINE__, track->number));

        if (track->type==bmedia_track_type_video && (track->info.video.codec == bvideo_codec_h264_svc || track->info.video.codec == bvideo_codec_h264_mvc)) {
            psi->extraVideoPid = track->number;
            psi->extraVideoCodec = B_PlaybackIp_UtilsVideoCodec2Nexus(track->info.video.codec);
            PRINTMSG_PROBE(("Extra video track %u codec:%u\n", track->number, psi->extraVideoCodec));
            continue;
        }
        else if (track->type==bmedia_track_type_video && !foundVideo) {
            PRINTMSG_PROBE(("Video track %u codec:%u\n", track->number, track->info.video.codec));
            psi->videoPid = track->number;
            psi->videoCodec = B_PlaybackIp_UtilsVideoCodec2Nexus(track->info.video.codec);
            psi->videoHeight = track->info.video.height;
            psi->videoWidth = track->info.video.width;
            foundVideo = true;
        }
        else if (track->type==bmedia_track_type_pcr) {  /* mp4 shouldn't have PCR track! */
            PRINTMSG_PROBE(("Pcr pid %u\n", track->number));
            psi->pcrPid = track->number;
        }
        else if(track->type==bmedia_track_type_audio && !foundAudio) {
            PRINTMSG_PROBE(("Audio track %u codec:%u\n", track->number, track->info.audio.codec));
            psi->audioPid = track->number;
            psi->audioCodec = B_PlaybackIp_UtilsAudioCodec2Nexus(track->info.audio.codec);
            foundAudio = true;
        }

        /* GARYWASHERE: Reorganize the following to fit with the
         * if-then-else above.  */
        switch(track->type) {
        case bmedia_track_type_audio:
            PRINTMSG_PROBE(("%s:%d : Found AUDIO track number: %u codec: %d", myFuncName, __LINE__, track->number, track->info.audio.codec));

            representationInfo->trackIdForAudio = track->number;

            if (bmp4_stream) {
                {
                    bmp4_mediaheaderbox     mediaheader;
                    B_PlaybackIp_MpegDashParseMdhdForTrackid(playback_ip, segmentBuffer, bmp4_track, track->number, &mediaheader);

                    representationInfo->mdhdTimescaleForAudio                     = mediaheader.timescale;
                    PRINTMSG_PROBE(("%s:%d: representationInfo->mdhdTimescaleForAudio=%lu", myFuncName, __LINE__, representationInfo->mdhdTimescaleForAudio));
                }

                {
                    bmp4_trackextendsbox    trackextends;

                    B_PlaybackIp_MpegDashParseTrexForTrackid(playback_ip, segmentBuffer, bmp4_stream, track->number, &trackextends);

                    representationInfo->trexDefaultSampleDescriptionIndexForAudio = trackextends.default_sample_description_index;
                    representationInfo->trexDefaultSampleDurationForAudio         = trackextends.default_sample_duration;
                    representationInfo->trexDefaultSampleFlagsForAudio            = trackextends.default_sample_flags;
                    representationInfo->trexDefaultSampleSizeForAudio             = trackextends.default_sample_size;

                    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleDescriptionIndexForAudio=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleDescriptionIndexForAudio));
                    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleDurationForAudio=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleDurationForAudio));
                    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleFlagsForAudio=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleFlagsForAudio));
                    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleSizeForAudio=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleSizeForAudio));
                }

                PRINTMSG_PROBE(("%s : %d : Audio codec: 0x%x  fourcc: "BMEDIA_FOURCC_FORMAT, myFuncName, __LINE__ , track->info.audio.codec, BMEDIA_FOURCC_ARG(representationInfo->fourccForAudio)  ));

                if (!B_PlaybackIp_MpegDashGetDecoderSpecificInfoForTrackid(
                        playback_ip,
                        segmentBuffer,
                        bmp4_track,
                        track->number,
                        &representationInfo->codecSpecificSizeForAudio,
                        (void *)&representationInfo->codecSpecificAddrForAudio,
                        &representationInfo->fourccForAudio))
                {
                    BDBG_ERR(("%s: Unable to obtain Decoder Specific Info for audio track ID: %u", myFuncName, track->number));
                    goto error;
                }

                PRINTMSG_PROBE_DUMP("Decoder Specific Info for Audio", "codecSpecificAddrForAudio", representationInfo->codecSpecificAddrForAudio, (unsigned)representationInfo->codecSpecificSizeForAudio);
            } /* end if (bmp4_stream) */
            break;

        case bmedia_track_type_video:
            PRINTMSG_PROBE(("%s:%d : Found VIDEO track number: %u codec: %d", myFuncName, __LINE__, track->number, track->info.video.codec));

            representationInfo->trackIdForVideo = track->number;

            if (bmp4_stream) {
                {
                    bmp4_mediaheaderbox     mediaheader;
                    B_PlaybackIp_MpegDashParseMdhdForTrackid(playback_ip, segmentBuffer, bmp4_track, track->number, &mediaheader);

                    representationInfo->mdhdTimescaleForVideo                     = mediaheader.timescale;
                    PRINTMSG_PROBE(("%s:%d: representationInfo->mdhdTimescaleForVideo=%lu", myFuncName, __LINE__, representationInfo->mdhdTimescaleForVideo));
                }

                {
                    bmp4_trackextendsbox    trackextends;

                    B_PlaybackIp_MpegDashParseTrexForTrackid(playback_ip, segmentBuffer, bmp4_stream, track->number, &trackextends);

                    representationInfo->trexDefaultSampleDescriptionIndexForVideo = trackextends.default_sample_description_index;
                    representationInfo->trexDefaultSampleDurationForVideo         = trackextends.default_sample_duration;
                    representationInfo->trexDefaultSampleFlagsForVideo            = trackextends.default_sample_flags;
                    representationInfo->trexDefaultSampleSizeForVideo             = trackextends.default_sample_size;
                    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleDescriptionIndexForVideo=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleDescriptionIndexForVideo));
                    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleDurationForVideo=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleDurationForVideo));
                    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleFlagsForVideo=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleFlagsForVideo));
                    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleSizeForVideo=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleSizeForVideo));
                }

                PRINTMSG_PROBE(("%s : %d : Video codec: 0x%x  fourcc: "BMEDIA_FOURCC_FORMAT, myFuncName, __LINE__ , track->info.video.codec, BMEDIA_FOURCC_ARG(representationInfo->fourccForVideo)));

                if (!B_PlaybackIp_MpegDashGetDecoderSpecificInfoForTrackid(
                        playback_ip,
                        segmentBuffer,
                        bmp4_track,
                        track->number,
                        &representationInfo->codecSpecificSizeForVideo,
                        (void *)&representationInfo->codecSpecificAddrForVideo,
                        &representationInfo->fourccForVideo))
                {
                    BDBG_ERR(("%s: Unable to obtain Decoder Specific Info for video track ID: %u", myFuncName, track->number));
                    goto error;
                }

                PRINTMSG_PROBE_DUMP("Decoder Specific Info for Video", "codecSpecificAddrForVideo", representationInfo->codecSpecificAddrForVideo, (unsigned)representationInfo->codecSpecificSizeForVideo);
            }
            break;

        case bmedia_track_type_other:
            PRINTMSG_PROBE(("%s:%d : Found OTHER track number: %u, ignoring...", myFuncName, __LINE__, track->number));
            break;

        default:
            break;
        }
    }

    PRINTMSG_PROBE(("%s:%d: psi->mpegType=%d", myFuncName, __LINE__, psi->mpegType));
    PRINTMSG_PROBE(("%s:%d: stream->type=%d", myFuncName, __LINE__, stream->type));
    PRINTMSG_PROBE(("%s:%d: psi->duration=%u", myFuncName, __LINE__, psi->duration));
    PRINTMSG_PROBE(("%s:%d: psi->avgBitRate=%u", myFuncName, __LINE__, psi->avgBitRate));
    PRINTMSG_PROBE(("%s:%d: psi->maxBufferDuration=%lu", myFuncName, __LINE__, psi->maxBufferDuration));
    PRINTMSG_PROBE(("%s:%d: psi->transportTimeStampEnabled=%d", myFuncName, __LINE__, psi->transportTimeStampEnabled));

    PRINTMSG_PROBE(("%s:%d: psi->extraVideoPid=%d", myFuncName, __LINE__, psi->extraVideoPid));
    PRINTMSG_PROBE(("%s:%d: psi->extraVideoCodec=%d", myFuncName, __LINE__, psi->extraVideoCodec));
    PRINTMSG_PROBE(("%s:%d: psi->videoPid=%d", myFuncName, __LINE__, psi->videoPid));
    PRINTMSG_PROBE(("%s:%d: psi->videoCodec=%d", myFuncName, __LINE__, psi->videoCodec));
    PRINTMSG_PROBE(("%s:%d: psi->videoHeight=%d", myFuncName, __LINE__, psi->videoHeight));
    PRINTMSG_PROBE(("%s:%d: psi->videoWidth=%d", myFuncName, __LINE__, psi->videoWidth));
    PRINTMSG_PROBE(("%s:%d: psi->pcrPid=%d", myFuncName, __LINE__, psi->pcrPid));
    PRINTMSG_PROBE(("%s:%d: psi->audioPid=%d", myFuncName, __LINE__, psi->audioPid));


    PRINTMSG_PROBE(("%s:%d: representationInfo->trackIdForAudio=%lu", myFuncName, __LINE__, representationInfo->trackIdForAudio));
    PRINTMSG_PROBE(("%s:%d: representationInfo->mdhdTimescaleForAudio=%lu", myFuncName, __LINE__, representationInfo->mdhdTimescaleForAudio));
    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleDescriptionIndexForAudio=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleDescriptionIndexForAudio));
    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleDurationForAudio=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleDurationForAudio));
    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleFlagsForAudio=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleFlagsForAudio));
    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleSizeForAudio=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleSizeForAudio));
    PRINTMSG_PROBE(("%s:%d: representationInfo->fourccForAudio="BMEDIA_FOURCC_FORMAT, myFuncName, __LINE__, BMEDIA_FOURCC_ARG(representationInfo->fourccForAudio)));
    PRINTMSG_PROBE_DUMP("Decoder Specific Info for Audio", "codecSpecificAddrForAudio", representationInfo->codecSpecificAddrForAudio, (unsigned)representationInfo->codecSpecificSizeForAudio);


    PRINTMSG_PROBE(("%s:%d: representationInfo->trackIdForVideo=%lu", myFuncName, __LINE__, representationInfo->trackIdForVideo));

    PRINTMSG_PROBE(("%s:%d: representationInfo->mdhdTimescaleForVideo=%lu", myFuncName, __LINE__, representationInfo->mdhdTimescaleForVideo));
    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleDescriptionIndexForVideo=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleDescriptionIndexForVideo));
    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleDurationForVideo=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleDurationForVideo));
    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleFlagsForVideo=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleFlagsForVideo));
    PRINTMSG_PROBE(("%s:%d: representationInfo->trexDefaultSampleSizeForVideo=%lu", myFuncName, __LINE__, representationInfo->trexDefaultSampleSizeForVideo));
    PRINTMSG_PROBE(("%s:%d: representationInfo->fourccForVideo="BMEDIA_FOURCC_FORMAT, myFuncName, __LINE__, BMEDIA_FOURCC_ARG(representationInfo->fourccForVideo)));
    PRINTMSG_PROBE_DUMP("Decoder Specific Info for Video", "codecSpecificAddrForVideo", representationInfo->codecSpecificAddrForVideo, (unsigned)representationInfo->codecSpecificSizeForVideo);

    psi->psiValid = true;
    representationInfo->psi = psi;  /* point to psi struct in playback_ip. */
    representationInfo->probeInfoIsValid = true;
    returnCode = true;  /* Indicate success! */
    /* Fall through to common cleanup code. */

error:
    if (probe && stream){bmedia_probe_stream_free(probe,stream);}
    if (probe) {bmedia_probe_destroy(probe);}
    if (fd2) {bfile_segment_buffer_read_detach(fd2);}
    if (mediaFileSegmentInfo){B_PlaybackIp_MpegDashFreeMediaFileSegmentInfo(mediaFileSegmentInfo);}

    return returnCode;
}


/*****************************************************************************
 * Given an MPD time, select the corresponding Period element.
 *****************************************************************************/
static MpegDashPeriodInfo *
B_PlaybackIp_MpegDashSelectPeriod(B_PlaybackIpHandle playback_ip, MpegDashMpdInfo *mpdInfo, uint64_t mpdSeekTimeInMs)
{
    MpegDashPeriodInfo    * periodInfo;
    MpegDashPeriodInfo    * prevPeriodInfo;
    MpegDashPeriodInfo    * chosenPeriodInfo = NULL;

    unsigned                periodIdx;

    uint64_t                thisPeriodStartInMs;
    uint64_t                thisPeriodDurationInMs;
    uint64_t                prevPeriodEndInMs = 0;
    uint64_t                mediaPresentationDurationInMs = 0;

    bool                    thisPeriodStartIsValid = false;
    bool                    thisPeriodDurationIsValid = false;
    bool                    prevPeriodEndIsValid = false;

    BSTD_UNUSED(playback_ip);
    BSTD_UNUSED(mpdSeekTimeInMs);

    PRINTMSG_SEEK(("SelectPeriod: Entry: seek mpdTimeInMs: %"PRIu64, mpdSeekTimeInMs));

    /* Iterate through Periods */
    prevPeriodInfo = NULL;
    for( periodIdx=0, periodInfo=BLST_Q_FIRST(&mpdInfo->mpegDashPeriodInfoHead) ;
         periodInfo ;
         periodIdx++, periodInfo=BLST_Q_NEXT(periodInfo, nextPeriodInfo))
    {
        thisPeriodStartIsValid = false;
        thisPeriodDurationIsValid = false;

        PRINTMSG_SEEK(("SelectPeriod: Checking Period: index: %d, id: %s start: %s duration: %s",
                       periodIdx, periodInfo->id, periodInfo->start, periodInfo->duration ));

        /* Try to get the start time for this period from the Period's "start"
         * attribute.  */
        if (periodInfo->cooked.start.valid)
        {
            if (B_PlaybackIp_MpegDashGetMsFromDuration(&periodInfo->cooked.start, &thisPeriodStartInMs))
            {
                thisPeriodStartIsValid = true;
                PRINTMSG_SEEK(("SelectPeriod:  Got period start from Period@start: %"PRIu64" ms", thisPeriodStartInMs));
            }
        }

        /* See if we can get the "duration" for the current Period.  We may
         * need it later if the next period doesn't have a "start" attrubute. */
        if (periodInfo->cooked.duration.valid)
        {
            if (B_PlaybackIp_MpegDashGetMsFromDuration(&periodInfo->cooked.duration, &thisPeriodDurationInMs))
            {
                thisPeriodDurationIsValid  = true;
                periodInfo->periodDurationInMs = thisPeriodDurationInMs;    /* Save the Period's duration in the Period object. */
                PRINTMSG_SEEK(("SelectPeriod:  Got period duration from Period@duration: %"PRIu64" ms", thisPeriodDurationInMs));


                PRINTMSG_SEEK(("SelectPeriod:  Setting duration for current period: %u %s to %"PRIu64" ms",
                               periodIdx, periodInfo->id, periodInfo->periodDurationInMs ));
            }
        }

        /* If that didn't work, see if we know the end of the previous period.
         * If we have it, then use that as the start of this period. */
        if (!thisPeriodStartIsValid && prevPeriodEndIsValid)
        {
            thisPeriodStartInMs = prevPeriodEndInMs;
            thisPeriodStartIsValid = true;
            PRINTMSG_SEEK(("SelectPeriod:  Got period start from previous Period's end: %"PRIu64" ms", thisPeriodStartInMs));
        }

        /* And if that didn't work, the last chance is to use zero, but that's
         * only allowed if this is the first period in a "static" MPD (refer
         * to Section 5.3.2.1. */
#if 0  /* ==================== GARYWASHERE - Start of Original Code ==================== */
        if (!thisPeriodStartIsValid &&  prevPeriodInfo == NULL && !mpdInfo->isDynamic)
#else  /* ==================== GARYWASHERE - Start of Modified Code ==================== */
                /* GARYWASHERE: For now, allow missing start time for the
                 * first period of a dynamic MPD...  because that's the way
                 * Harmonic is doing it for now.  */
        if (!thisPeriodStartIsValid &&  prevPeriodInfo == NULL )
#endif /* ==================== GARYWASHERE - End of Modified Code   ==================== */
        {
            thisPeriodStartInMs = 0;
            thisPeriodStartIsValid = true;
            PRINTMSG_SEEK(("SelectPeriod:  Period start unspecified, setting to zero for first period: %"PRIu64" ms", thisPeriodStartInMs));
        }

        if (!thisPeriodStartIsValid )
        {
            BDBG_WRN(("%s:%d : Skipping period: index: %d, id: %s. Can't determine start time.", __FUNCTION__, __LINE__, periodIdx, periodInfo->id ));
        }

        /* We may have chosen a Period from a prior iteration, but it might
         * not be the correct one.  If this Period has a starting time that's
         * before the seek time, then disregard the previously chosen
         * Period and use this one instead. */
        if (thisPeriodStartIsValid)
        {
            /* Since we know the start time (in ms) for this Period, save it
             * in the Period object so we can refer to it later (if we happen
             * to select this Period).  */

            periodInfo->periodStartInMs = thisPeriodStartInMs;

            /* Also, since we know the start time of this period, try to
             * figure out the duration of the previous period by subtracting
             * the previous period's start time.  */
            if (prevPeriodInfo) {
                if (prevPeriodInfo->periodDurationInMs == 0) {
                    prevPeriodInfo->periodDurationInMs = thisPeriodStartInMs - prevPeriodInfo->periodStartInMs;
                    PRINTMSG_SEEK(("SelectPeriod:  Setting duration for prev period: %u %s to %"PRIu64" ms",
                                   periodIdx-1, prevPeriodInfo->id, prevPeriodInfo->periodDurationInMs ));
                }
            }

            if (thisPeriodStartInMs > mpdSeekTimeInMs)  /* Period starts after seek time. */
            {
                break;  /* This period starts too late.  Use the previous period if we chose it. */
            }
            else  /* This Period starts before (or at) the seek time. */
            {
                if (thisPeriodDurationIsValid && thisPeriodStartInMs + thisPeriodDurationInMs < mpdSeekTimeInMs)
                {
                    chosenPeriodInfo = NULL;    /* nope, Period is too short.  */
                    PRINTMSG_SEEK(("SelectPeriod:  Period too short, disregarding Period: index: %d, id: %s start: %s duration: %s",
                                   periodIdx, periodInfo->id, periodInfo->start, periodInfo->duration ));
                }
                else {
                    chosenPeriodInfo = periodInfo;  /* might be able to use this Period. */

                    PRINTMSG_SEEK(("SelectPeriod:  Possible Period: index: %d, id: %s start: %s duration: %s",
                                   periodIdx, periodInfo->id, periodInfo->start, periodInfo->duration ));
                }
            }
        }

        /* Compute the end of this Period (if we can) and save it for the next
         * time through the loop. */
        prevPeriodEndIsValid = false;

        if (thisPeriodStartIsValid && thisPeriodDurationIsValid)
        {
            prevPeriodEndInMs = thisPeriodStartInMs + thisPeriodDurationInMs;
            prevPeriodEndIsValid = true;
        }

        prevPeriodInfo = periodInfo;
    }

    if (!chosenPeriodInfo) goto error;

    /* If the chosen Period doesn't have a duration, then we can try to use
     * the MPD@mediaPresentationDuration as the Period's end, then compute the
     * duration with that.  */
    if (chosenPeriodInfo->periodDurationInMs == 0) {            /* We don't know this Period's duration. */
        if (mpdInfo->cooked.mediaPresentationDuration.valid)    /* If MPD has a MediaPresentationDuration... */
        {                                                       /* Convert duration to milliseconds. */
            if (B_PlaybackIp_MpegDashGetMsFromDuration(&mpdInfo->cooked.mediaPresentationDuration, &mediaPresentationDurationInMs))
            {
                chosenPeriodInfo->periodDurationInMs = mediaPresentationDurationInMs - chosenPeriodInfo->periodStartInMs;

                PRINTMSG_SEEK(("SelectPeriod:  Using MPD@mediaPresentationDuration to set Period duration to :%"PRIu64" ms",
                               chosenPeriodInfo->periodDurationInMs));
            }
        }
    }

    /* Now for one last final check to make sure that the chosen
     * period includes the MPD seek time.  */
    if ((mpdSeekTimeInMs < chosenPeriodInfo->periodStartInMs))
    {
        BDBG_ERR(("SelectPeriod: Seek is before chosen Period! mpdSeekTimeInMs: %"PRIu64" periodStartInMs: %"PRIu64, mpdSeekTimeInMs, chosenPeriodInfo->periodStartInMs));
        goto error;
    }

    if ((chosenPeriodInfo->periodDurationInMs >0 &&
         mpdSeekTimeInMs >= chosenPeriodInfo->periodStartInMs + chosenPeriodInfo->periodDurationInMs))
    {
        BDBG_ERR(("SelectPeriod: Seek is after chosen Period! mpdSeekTimeInMs: %"PRIu64" periodEndInMs: %"PRIu64,
                  mpdSeekTimeInMs, chosenPeriodInfo->periodStartInMs + chosenPeriodInfo->periodDurationInMs ));
        goto error;
    }


    PRINTMSG_SEEK(("SelectPeriod: Final Period choice: id: %s startInMs: %"PRIu64" durationInMs: %"PRIu64"",
                   chosenPeriodInfo->id, chosenPeriodInfo->periodStartInMs, chosenPeriodInfo->periodDurationInMs ));


    BDBG_NUL(("SelectPeriod: Final Period choice: id: %s startInMs: %"PRIu64" durationInMs: %"PRIu64"",
              chosenPeriodInfo->id,
              chosenPeriodInfo->periodStartInMs,
              chosenPeriodInfo->periodDurationInMs ));

    return chosenPeriodInfo;

error:
    BDBG_WRN(("SelectPeriod: No Period found for seek time of %"PRIu64" ms", mpdSeekTimeInMs ));
    return NULL;
}


/*****************************************************************************
 * Given a Period element, select AdaptationSets to use for audio and video.
 * The audio and video might combined in a single AdaptationSet, or they might
 * be in two different ones.
 *****************************************************************************/
static bool
B_PlaybackIp_MpegDashSelectAdaptationSets(B_PlaybackIpHandle playback_ip,
                                          MpegDashPeriodInfo * periodInfo,
                                          MpegDashAdaptationSetInfo **videoAdaptationSet,
                                          MpegDashAdaptationSetInfo **audioAdaptationSet)
{
    MpegDashAdaptationSetInfo    * adaptationSetInfo;
    MpegDashContentComponentInfo * contentComponentInfo;
    BSTD_UNUSED(playback_ip);

    *audioAdaptationSet = NULL;
    *videoAdaptationSet = NULL;

    /* Iterate through AdaptationSets */
    for(adaptationSetInfo = BLST_Q_FIRST(&periodInfo->mpegDashAdaptationSetInfoHead) ; adaptationSetInfo ; adaptationSetInfo = BLST_Q_NEXT(adaptationSetInfo, nextAdaptationSetInfo))
    {
        const char * mimeType    = NULL;
        const char * contentType = NULL;

        PRINTMSG_SEEK(("SelectAdaptationSet: Checking AdaptationSet id:%s, mimeType:%s startsWithSAP:%u segmentAlignment:%u at %p",
                   adaptationSetInfo->id,   adaptationSetInfo->baseTypeRepresentationBase->mimeType,
                   adaptationSetInfo->baseTypeRepresentationBase->startWithSAP,
                   adaptationSetInfo->segmentAlignment,     (void *)adaptationSetInfo ));

        /* GARYWASHERE: This will need to be more general... checking contentType and contentComponent.
         * and codecs to insure that we can decode the content in the AdaptationSet.  It will probably
         * be necessary to look through the Representations to find the codec and other details about
         * the content.  */

        mimeType = adaptationSetInfo->baseTypeRepresentationBase->mimeType;
        if (mimeType) {
            if (strstr(mimeType, XML_VAL_CONTENT_TYPE_AUDIO) == mimeType) {
                *audioAdaptationSet = adaptationSetInfo;
                adaptationSetInfo->hasAudio = true;
                PRINTMSG_SEEK(("SelectAdaptationSet:  Found audio mimeType for AdaptationSet at %p", (void *)adaptationSetInfo));
            }

            if (strstr(mimeType, XML_VAL_CONTENT_TYPE_VIDEO) == mimeType) {
                *videoAdaptationSet = adaptationSetInfo;
                adaptationSetInfo->hasVideo = true;
                PRINTMSG_SEEK(("SelectAdaptationSet:  Found video mimeType for AdaptationSet at %p", (void *)adaptationSetInfo));
            }
        }
        else /* no mimeType in the AdaptationSet... take a look in each of the Representations. */
        {
            MpegDashRepresentationInfo * representationInfo;

            /* Iterate through Representations in this AdaptationSet */
            for(representationInfo = BLST_Q_FIRST(&adaptationSetInfo->mpegDashRepresentationInfoHead) ; representationInfo ; representationInfo = BLST_Q_NEXT(representationInfo, nextRepresentationInfo))
            {
                PRINTMSG_SEEK(("SelectAdaptationSet:  Checking Representation: id:%s",
                                representationInfo->id ));

                mimeType = representationInfo->baseTypeRepresentationBase->mimeType;
                if (mimeType) {
                    if (strstr(mimeType, XML_VAL_CONTENT_TYPE_AUDIO) == mimeType) {
                        *audioAdaptationSet = adaptationSetInfo;
                        adaptationSetInfo->hasAudio = true;
                        PRINTMSG_SEEK(("SelectAdaptationSet:   Found audio mimeType in AdaptationSet's Representation at %p", (void *)representationInfo));
                    }

                    if (strstr(mimeType, XML_VAL_CONTENT_TYPE_VIDEO) == mimeType) {
                        *videoAdaptationSet = adaptationSetInfo;
                        adaptationSetInfo->hasVideo = true;
                        PRINTMSG_SEEK(("SelectAdaptationSet:   Found video mimeType in AdaptationSet's Representation at %p", (void *)representationInfo));
                    }
                }
            }
        }

        /* Iterate through any ContentComponents that might belong to the AdaptationSet */
        for(contentComponentInfo = BLST_Q_FIRST(&adaptationSetInfo->mpegDashContentComponentInfoHead) ; contentComponentInfo ; contentComponentInfo = BLST_Q_NEXT(contentComponentInfo, nextContentComponentInfo))
        {
            PRINTMSG_SEEK((  "Found ContentComponent:  " MPEG_DASH_CONTENTCOMPONENT_TO_PRINTF_ARGS(contentComponentInfo)  ));

            contentType = contentComponentInfo->contentType;
            if (contentType)
            {
                if (strcmp(contentType, XML_VAL_CONTENT_TYPE_AUDIO) == 0) {
                    *audioAdaptationSet = adaptationSetInfo;
                    adaptationSetInfo->hasAudio = true;
                    PRINTMSG_SEEK(("SelectAdaptationSet:   Found audio contentType for AdaptationSet at %p", (void *)adaptationSetInfo));
                }

                if (strcmp(contentType, XML_VAL_CONTENT_TYPE_VIDEO) == 0 ) {
                    *videoAdaptationSet = adaptationSetInfo;
                    adaptationSetInfo->hasVideo = true;
                    PRINTMSG_SEEK(("SelectAdaptationSet:   Found video contentType for AdaptationSet at %p", (void *)adaptationSetInfo));
                }
            } /* End if contentType is present */
        } /* End for each ContentComponent in AdaptationSet */
    } /* End for each AdaptationSet */

    return true;
}

/*****************************************************************************
 * Given an AdaptationSet, select the appropriate Representation based on the
 * current network bandwidth and the bandwidth required by each
 * Representation.
 *****************************************************************************/
static MpegDashRepresentationInfo *
B_PlaybackIp_MpegDashSelectRepresentation(B_PlaybackIpHandle playback_ip, MpegDashAdaptationSetInfo * adaptationSetInfo, unsigned availableNetworkBandwidthInKbps)
{
    MpegDashRepresentationInfo * representationInfo;
    MpegDashRepresentationInfo * bestRepresentationInfo    = NULL;
    MpegDashRepresentationInfo * slowestRepresentationInfo = NULL;
    BSTD_UNUSED(playback_ip);

    unsigned                    availableNetworkBandwidth = availableNetworkBandwidthInKbps * 1000;

    BDBG_NUL(("%s:%d: Entry: availableNetworkBandwidth=%u", __FUNCTION__, __LINE__, availableNetworkBandwidth));
    /* Iterate through Representations in this AdaptationSet */
    for(representationInfo = BLST_Q_FIRST(&adaptationSetInfo->mpegDashRepresentationInfoHead) ; representationInfo ; representationInfo = BLST_Q_NEXT(representationInfo, nextRepresentationInfo))
    {
        PRINTMSG_SEEK(("SelectRepresentation: Checking Representation: id:%s bandwidth:%u at %p",
                        representationInfo->id, representationInfo->bandwidthNumeric, (void *)representationInfo ));

        /* First, just keep track of the slowest Representation (the one with
         * the lowest bandwidth requirements.  */
        if (!slowestRepresentationInfo) {
            slowestRepresentationInfo = representationInfo;
        }
        else if (representationInfo->bandwidthNumeric < slowestRepresentationInfo->bandwidthNumeric ) {
            slowestRepresentationInfo = representationInfo;
        }

        /*  Then, keep track of the Representation with the highest bandwidth
         *  that doesn't exceed the maximum allowed bandwidth.  */
        if (representationInfo->bandwidthNumeric > availableNetworkBandwidth)
        {
            PRINTMSG_SEEK(("SelectRepresentation:  Bandwidth %u is beyond available of %u",
                        representationInfo->bandwidthNumeric, availableNetworkBandwidth ));
        }
        else if (representationInfo->bandwidthNumeric <= (bestRepresentationInfo ? bestRepresentationInfo->bandwidthNumeric : 0) )
        {
            PRINTMSG_SEEK(("SelectRepresentation:  Bandwidth %u is not as high as previous representation's: %u",
                        representationInfo->bandwidthNumeric, (bestRepresentationInfo ? bestRepresentationInfo->bandwidthNumeric : 0) ));
        }
        else
        {
            if (bestRepresentationInfo) {
                PRINTMSG_SEEK(("SelectRepresentation:  Disregarding previous best Representation: id:%s bandwidth:%u at %p",
                        bestRepresentationInfo->id, bestRepresentationInfo->bandwidthNumeric, (void *)bestRepresentationInfo ));
            }

            PRINTMSG_SEEK(("SelectRepresentation:  Best Representation so far is now: id:%s bandwidth:%u at %p",
                        representationInfo->id, representationInfo->bandwidthNumeric, (void *)representationInfo ));

            bestRepresentationInfo = representationInfo;
        }
    }

    /* If none of the Representations are below the available
     * bandwidth, then just use the slowest one.  */
    if (!bestRepresentationInfo) {
        PRINTMSG_SEEK(("SelectRepresentation:  No Representation fits available bandwidth:%u, selecting slowest: id:%s bandwidth:%u at %p",
                        availableNetworkBandwidth,  slowestRepresentationInfo->id, slowestRepresentationInfo->bandwidthNumeric, (void *)slowestRepresentationInfo ));
        bestRepresentationInfo = slowestRepresentationInfo;
    }

    if ( ! bestRepresentationInfo)
    {
        BDBG_ERR(("%s: Can't find any usable Representations under AdaptationSet element", __FUNCTION__));
    }
    else
    {
        PRINTMSG_SEEK(("SelectRepresentation: Final choice for Representation: id:%s bandwidth:%u at %p",
                        bestRepresentationInfo->id, bestRepresentationInfo->bandwidthNumeric, (void *)bestRepresentationInfo ));
    }

    return bestRepresentationInfo;
}


/*****************************************************************************
 * Given both audio and video AdaptationSets, select the appropriate
 * audio and video Representations based on the current network bandwidth
 * and the bandwidth required by each Representation.
 *
 * Note that audio and video may be in two distinct AdaptationSets or might
 * both be in the same one. *
 *****************************************************************************/
static bool
B_PlaybackIp_MpegDashSelectRepresentations( B_PlaybackIpHandle playback_ip,
                                            MpegDashAdaptationSetInfo  * audioAdaptationSet,
                                            MpegDashAdaptationSetInfo  * videoAdaptationSet,
                                            MpegDashRepresentationInfo ** newAudioRepresentationInfo,
                                            MpegDashRepresentationInfo ** newVideoRepresentationInfo,
                                            unsigned availableBandwidthInKbps)
{
    MpegDashRepresentationInfo * audioRepresentationInfo = NULL;
    MpegDashRepresentationInfo * videoRepresentationInfo = NULL;

#if 0  /* ==================== GARYWASHERE - Start of Original Code ==================== */
        /* This is some unit test code for exercising Representation switching. */
    {
        static int count = 0;

        count++;


        /* 1000 => Pid 1 (Medium/Low Quality) */
        /*  300 => Pid 2 (Low Quality)        */
        /* 2000 => Pid 3 (High Quality)       */

        if (count  < 12) {      /* From 0 to 7 secs, Starz logo and Cars 2 title screen. */
            availableBandwidthInKbps = 1000;
            BDBG_LOG(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
        }
        else if (count  < 18) { /* From 7 to 14 secs, Finn McMissile swings down, drives around, ends when he meets the green Gremlin. */
            availableBandwidthInKbps = 300;
            BDBG_LOG(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
        }
        else if (count  < 32) { /* From 14 to 28 secs, Finn chases Gremlin, ends with Gremlin about to fly out of the parking garage. */
            availableBandwidthInKbps = 2000;
            BDBG_LOG(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
        }
        else if (count  < 38) { /* After 28 seconds, starts with Gremlin flying out of the parking garage, Finn continues up, releases barrels, things explode. */
            availableBandwidthInKbps = 300;
            BDBG_LOG(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
        }
        else if (count  < 50) {
            availableBandwidthInKbps = 300;
            BDBG_LOG(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
        }
        else if (count  < 60) {
            availableBandwidthInKbps = 300;
            BDBG_LOG(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
        }
        else {
            availableBandwidthInKbps = 300;
            BDBG_LOG(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
        }
    }
#elif 0  /* ==================== GARYWASHERE - Start of Modified Code ==================== */

    {
        static int count = 0;

        count++;


        /* 1000 => Pid 1 (Medium/Low Quality) */
        /*  300 => Pid 2 (Low Quality)        */
        /* 2000 => Pid 3 (High Quality)       */


        if (count%12 < 4 ) {
            availableBandwidthInKbps = 1000;
            BDBG_ERR(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
        }
        else if (count%12 < 8) {
            availableBandwidthInKbps = 1000;
            BDBG_ERR(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
        }
        else {
            availableBandwidthInKbps = 300;
            BDBG_ERR(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
        }
    }

#elif 0  /* ==================== GARYWASHERE - Start of Modified Code ==================== */

    {
        static int count = 0;
        count++;

        if (count <= 4) {
            availableBandwidthInKbps = 7200;
            BDBG_ERR(("%s : %d : Overriding BW with %d Kbps (count:%u)", __FUNCTION__, __LINE__, availableBandwidthInKbps, count  ));
        }
        else if (count <= 5) {
            availableBandwidthInKbps = 2400;
            BDBG_ERR(("%s : %d : Overriding BW with %d Kbps (count:%u)", __FUNCTION__, __LINE__, availableBandwidthInKbps, count  ));
        }
        else {
            availableBandwidthInKbps = 2400;
            BDBG_ERR(("%s : %d : Overriding BW with %d Kbps (count:%u)", __FUNCTION__, __LINE__, availableBandwidthInKbps, count  ));
        }
    }

#elif 0  /* ==================== GARYWASHERE - Start of Modified Code ==================== */

    {
            availableBandwidthInKbps =  5000;  /* badminton 1920x1080 */
            availableBandwidthInKbps = 10000;  /* badminton 2560x1440 */
            availableBandwidthInKbps = 22000;  /* badminton 3200x1800 */
            availableBandwidthInKbps = 30000;  /* badminton 3840x2160 */
            BDBG_ERR(("%s : %d : Overriding BW with %d Kbps ", __FUNCTION__, __LINE__, availableBandwidthInKbps   ));
    }

#endif /* ==================== GARYWASHERE - End of Modified Code   ==================== */

    /* We have a few different cases depending on the combination of audio
     * and/or video AdaptationSets.  Handle each one separately. */

    if (!audioAdaptationSet && !videoAdaptationSet) {
        /* If neither audio nor video, there's nothing we can do, must be an error. */
        BDBG_ERR(("%s: No audio or video representation... Can't seek!", __FUNCTION__));
        goto selectError;
    }
    else if (audioAdaptationSet && !videoAdaptationSet) {
        /* If only audio, choose an audio Representation using all available bandwidth. */
        audioRepresentationInfo = B_PlaybackIp_MpegDashSelectRepresentation(playback_ip, audioAdaptationSet, availableBandwidthInKbps);
        if (!audioRepresentationInfo) {
            BDBG_ERR(("%s: Can't find any usable Audio Representations", __FUNCTION__));
            goto selectError;
        }
    }
    else if (!audioAdaptationSet && videoAdaptationSet) {
        /* If only video, choose a video Representation using all available bandwidth. */
        videoRepresentationInfo = B_PlaybackIp_MpegDashSelectRepresentation(playback_ip, videoAdaptationSet, availableBandwidthInKbps);
        if (!videoRepresentationInfo) {
            BDBG_ERR(("%s: Can't find any usable Audio Representations", __FUNCTION__));
            goto selectError;
        }
    }
    else if (audioAdaptationSet && videoAdaptationSet) {
        /* If we have video and audio, they might be the same
         * AdaptationSet, or two different ones. */
        if (audioAdaptationSet == videoAdaptationSet) {
            /* Audio and video in the same AdaptationSet, just select one
             * Representation from it and use it for both. */
            videoRepresentationInfo = B_PlaybackIp_MpegDashSelectRepresentation(playback_ip, videoAdaptationSet, availableBandwidthInKbps);
            if (!videoRepresentationInfo) {
                BDBG_ERR(("%s: Can't find any usable Audio Representations", __FUNCTION__));
                goto selectError;
            }
            audioRepresentationInfo = videoRepresentationInfo;
        }
        else {
            /* Audio and video are in different AdaptationSets, choose an
             * audio Representation using a fraction of the available bandwidth,
             * the choose a video representation with whatever bandwidth is left
             * over.*/
            unsigned audioBandwidthInKbps = availableBandwidthInKbps / 10;   /* allow 10% for audio */

            audioRepresentationInfo = B_PlaybackIp_MpegDashSelectRepresentation(playback_ip, audioAdaptationSet, audioBandwidthInKbps);
            if (!audioRepresentationInfo) {
                BDBG_ERR(("%s: Can't find any usable audio Representations", __FUNCTION__));
                goto selectError;
            }

            if (audioRepresentationInfo->bandwidthNumeric < availableBandwidthInKbps*1000) {
                availableBandwidthInKbps -= audioRepresentationInfo->bandwidthNumeric/1000;
            }
            else {
                availableBandwidthInKbps = 0;
            }

            /* And if audio and video are in different AdaptationSets, then find the best
             * audio Representation from it's AdaptationSet.   */
            videoRepresentationInfo = B_PlaybackIp_MpegDashSelectRepresentation(playback_ip, videoAdaptationSet, availableBandwidthInKbps);
            if (!videoRepresentationInfo) {
                BDBG_ERR(("%s: Can't find any usable video Representations", __FUNCTION__));
                goto selectError;
            }
        }
    }

    *newAudioRepresentationInfo = audioRepresentationInfo;
    *newVideoRepresentationInfo = videoRepresentationInfo;

    return true;  /* Normal success. */

selectError:
    return false;
}


/*****************************************************************************
 * Just a simple stub function which can be used to determine if a BDBG
 * logging is enabled.
 *****************************************************************************/
static const char  *B_PlaybackIp_MpegDash_P_CheckBdbgEnabled(bool *enabled) {
    *enabled = true;
     return "";
}


/*****************************************************************************
 * Given a pointer to the downloaded MPD file (in XML format), parse the XML
 * into tree of XML objects (actually B_PlaybackIp_XmlElements). Then use
 * that tree to create a tree of C-based objects that correspond to each of
 * the MPD's data types.
 *****************************************************************************/
static bool
B_PlaybackIp_MpegDashParseMpd(B_PlaybackIpHandle playback_ip, MpegDashMpdInfo **pMpdInfo)
{
    MpegDashSessionState *mpegDashSession = playback_ip->mpegDashSessionState;

    B_PlaybackIp_XmlElement   xmlElemRoot                   = NULL;
    B_PlaybackIp_XmlElement   xmlElemMpd                    = NULL;
    MpegDashMpdInfo             * mpdInfo                   = NULL;

    BDBG_ENTER(B_PlaybackIp_MpegDashParseMpd);

                                                                                                                                         /* To determine if our debuglevel is enabled, we'll need to make two calls to BDBG_MODULE_MSG.  The first call
     * is to make sure that the module gets registered, then the second will only evaluate the argument list
       if the BDBG level is enabled. */
    PRINTMSG_XML(("b_playback_ip_mpeg_dash_xml is enabled"));   /* Make sure that bape_mixerprint is registered with BDBG   */
    PRINTMSG_XML(("Printing MPD's XML information...%s", B_PlaybackIp_MpegDash_P_CheckBdbgEnabled(&g_xmlDebugEnabled)));  /* Set dbug_enabled if b_playback_ip_mpeg_dash_xml is enabled           */

    /* Parse the XML string and create a tree of XML objects.  This tree will
       need to be freed by calling B_PlaybackIp_Xml_Destroy( xmlElemRoot */
    xmlElemRoot = B_PlaybackIp_Xml_Create(mpegDashSession->mpdParseBuffer);
    mpegDashSession->xmlElemRoot = xmlElemRoot; /* save in SessionState struct */
    PRINT_XML_DEBUG(("XML Root Node: %p", (void *)xmlElemRoot), xmlElemRoot );
    if (!xmlElemRoot)
    {
        BDBG_ERR(("%s: XML Parser Failed to parse the MPD file", __FUNCTION__));
        goto mpd_parse_failed;
    }

    /* Get the pointer to the "MPD" element under the root.*/
    xmlElemMpd = B_PlaybackIp_XmlElem_FindChild(xmlElemRoot , XML_TAG_MPD);
    PRINT_XML_DEBUG(("MPD Element: %p", (void *)xmlElemMpd), xmlElemMpd );
    if (!xmlElemMpd)
    {
        BDBG_ERR(("%s: Can't find top-level MPD element", __FUNCTION__));
        goto mpd_parse_failed;
    }

    /* Now create the C-based object tree from the XML tree.  This will create objects
     * for the MPD and all of its subelements. */
    mpdInfo = B_PlaybackIp_MpegDashCreateMpd(playback_ip, xmlElemMpd);
    if (!mpdInfo) goto mpd_parse_failed;

    B_PlaybackIp_MpegDashGetDateTimeNow(&mpdInfo->fetchTime);

    PRINTMSG_SEEK(( MPEG_DASH_MPD_TO_PRINTF_ARGS(mpdInfo)  ));

    *pMpdInfo = mpdInfo;    /* Give caller the pointer to the new MPD object. */

    PRINTMSG_XML(("%s: finished parsing MPD file", __FUNCTION__));
    BDBG_LEAVE(B_PlaybackIp_MpegDashParseMpd);
    return true;    /* true => success return code */

mpd_parse_failed:
    BDBG_ERR(("%s: Abandoning MPD processing!", __FUNCTION__));
    /* fall through to "error:" return. */

    BDBG_LEAVE(B_PlaybackIp_MpegDashParseMpd);
    return false;   /* false => failure return code */
}


/*****************************************************************************
 * Given a PeriodTime and a SegmentTimeline, this function will
 * lookup the PeriodTime and build a virtual/fake
 * SegmentTimelineElem that describes the segment that contains
 * the PeriodTime.
 *****************************************************************************/
static void
B_PlaybackIp_MpegDashSegmentTimelineLookup(
            B_PlaybackIpHandle                 playback_ip,
            MpegDashSegmentTimelineInfo      * segmentTimeline,
            uint64_t                           periodTimeInTimescaleUnits,
            MpegDashSegmentTimelineElemInfo  * resultSegmentTimelineElem) /* put the results here */
{
    MpegDashSegmentTimelineElemInfo     * segmentTimelineElem;
    uint64_t        elemsPeriodTimeInTimescaleUnits = 0;
    uint64_t        elemsEptInTimescaleUnits = 0; /* Ept -> earliest_presentation_time */
    unsigned long   elemsSegmentNumber = 0;
    unsigned long   elemsRepeatCount;
    unsigned long   elemsDuration;

    BSTD_UNUSED(playback_ip);

    BKNI_Memset(resultSegmentTimelineElem, 0, sizeof (*resultSegmentTimelineElem));

    resultSegmentTimelineElem->parsed = false;  /* Indicate failure (until we succeed) */

    /* Iterate through each element in the SegmentTimeline. */
    segmentTimelineElem = NULL;
    for( segmentTimelineElem=BLST_Q_FIRST(&segmentTimeline->mpegDashSegmentTimelineElemInfoHead);
         segmentTimelineElem;
         segmentTimelineElem=BLST_Q_NEXT(segmentTimelineElem, nextSegmentTimelineElemInfo))
    {
        /* If "t" is specified, set it as the current EPT.  Otherwise
         * just leave it and carry it over from the last iteration (or
         * from zero). */
        if (segmentTimelineElem->t) {
            elemsEptInTimescaleUnits = segmentTimelineElem->cooked.t;
        }

        /* Get a local copy of the repeat count. Use zero if not
         * specified. */
        elemsRepeatCount = segmentTimelineElem->r ? segmentTimelineElem->cooked.r : 0;
        elemsDuration    = segmentTimelineElem->cooked.d * (elemsRepeatCount + 1);

        /* Save the accumulated values into the current element. */
        segmentTimelineElem->segmentTimeInTimescaleUnits = elemsPeriodTimeInTimescaleUnits;
        segmentTimelineElem->segmentEptInTimescaleUnits  = elemsEptInTimescaleUnits;
        segmentTimelineElem->segmentNumber               = elemsSegmentNumber;
        segmentTimelineElem->parsed                      =  true;  /* indicates that the fields are populated */


        BDBG_NUL(("Iterating...  "  MPEG_DASH_SEGMENTTIMELINEELEM_TO_PRINTF_ARGS(segmentTimelineElem)   ));

        BDBG_NUL(("%s:%d : SegmentNumber: %ld  PeriodTime: %"PRIu64"  EPT: %"PRIu64"", __FUNCTION__, __LINE__ , elemsSegmentNumber, elemsPeriodTimeInTimescaleUnits, elemsEptInTimescaleUnits   ));

        /* Check to see if this element spans the period time we're
         * looking for. */
        if (periodTimeInTimescaleUnits < elemsPeriodTimeInTimescaleUnits + elemsDuration)  break;

        /* Set up for the next iteration.  Add the current element's
         * duration to the current element's EPT and Period time. */
        elemsEptInTimescaleUnits        += segmentTimelineElem->cooked.d * (elemsRepeatCount + 1);
        elemsPeriodTimeInTimescaleUnits += segmentTimelineElem->cooked.d * (elemsRepeatCount + 1);
        elemsSegmentNumber += elemsRepeatCount + 1;
    }

    if (!segmentTimelineElem)
    {
        BDBG_ERR(("Can't find PeriodTime: %"PRIu64" in SegmentTimeline", periodTimeInTimescaleUnits ));
        (void)BERR_TRACE(B_ERROR_UNKNOWN);  /* GARYWASHERE: Maybe this should be handled as EOF??? */
        goto seek_failed;
    }

    /* Okay, so now we have the SegmentTimelineElem that we need.
     * This Elem represents one or more segments, so we need to
     * figure out which of those segments our periodTime is in. */
    {
        uint64_t        timeInElem = periodTimeInTimescaleUnits - elemsPeriodTimeInTimescaleUnits;
        unsigned long   segmentOffset;

        /* Compute the distance in Segments that the periodTime is
         * from the start of this Elem.  */
        segmentOffset = timeInElem / segmentTimelineElem->cooked.d;

        /* Now just fill in the results structure. */
        resultSegmentTimelineElem->segmentTimeInTimescaleUnits = elemsPeriodTimeInTimescaleUnits + (segmentOffset * segmentTimelineElem->cooked.d);
        resultSegmentTimelineElem->segmentEptInTimescaleUnits  = elemsEptInTimescaleUnits + (segmentOffset * segmentTimelineElem->cooked.d);
        resultSegmentTimelineElem->segmentNumber               = elemsSegmentNumber + segmentOffset;
        resultSegmentTimelineElem->cooked.d                    = segmentTimelineElem->cooked.d;
        resultSegmentTimelineElem->cooked.t                    = resultSegmentTimelineElem->segmentEptInTimescaleUnits;
        resultSegmentTimelineElem->cooked.r                    = 0; /* because this is just a fake SegmentTimelineElem. */
    }

    resultSegmentTimelineElem->parsed = true;
    return;

seek_failed:
    resultSegmentTimelineElem->parsed = false;
    return;
}


/*****************************************************************************
 * Given a Representation and PeriodTime (specified in the Representation's
 * timescale units), update a RepresentationSeekContext
 * with information needed to access the segments for that time.
 *****************************************************************************/
static B_PlaybackIpError
B_PlaybackIp_MpegDashSeekInRepresentationByTimescale(
                                B_PlaybackIpHandle                  playback_ip,
                                MpegDashRepresentationInfo        * representationInfo,
                                MpegDashRepresentationSeekContext * representationSeekCtx,
                                uint64_t                            periodTimeInTimescaleUnits)
{
    BSTD_UNUSED(playback_ip);

    MpegDashMpdInfo               * mpdInfo;

    MpegDashSegmentBaseInfo       * segmentBase;
    MpegDashSegmentListInfo       * segmentList;
    MpegDashSegmentTemplateInfo   * segmentTemplate;

    uint64_t                        periodTimeInMs = 0;

    B_PlaybackIpError               errCode = B_ERROR_UNKNOWN;
    unsigned long                   timescale = representationInfo->inheritedTimescale;
    char                          * myFuncName = "SeekInReprByTs";

    periodTimeInMs = (periodTimeInTimescaleUnits * 1000 ) / timescale;
    PRINTMSG_SEEK(("%s:%d: timescale=%lu", myFuncName, __LINE__, timescale));
    PRINTMSG_SEEK(("%s:%d: periodTimeInTimescaleUnits="BDBG_UINT64_FMT, myFuncName, __LINE__, BDBG_UINT64_ARG(periodTimeInTimescaleUnits)));
    PRINTMSG_SEEK(("%s:%d : Seeking to periodTimeInMs=%"PRIu64"", myFuncName, __LINE__  , periodTimeInMs));

    mpdInfo = representationInfo->parentObj->parentObj->parentObj;

    /*------------------------------------------------------------
     * Check to make sure that the seek time (which is relative to
     * the start of the Period) is actually within the Period.
     *----------------------------------------------------------*/
    if (mpdInfo->isDynamic) {
        MpegDashDateTime  nowDateTime;
        MpegDashDuration  durationSinceMpdAvailable;
        uint64_t          mostRecentContentTimeInMs;

        /*  Get the time of the most recent content (which is NOW). */
         B_PlaybackIp_MpegDashGetDateTimeNow(&nowDateTime);
         B_PlaybackIp_MpegDashGetDateTimeDifference(&nowDateTime, &mpdInfo->cooked.availabilityStartTime, &durationSinceMpdAvailable);
         B_PlaybackIp_MpegDashGetMsFromDuration(&durationSinceMpdAvailable, &mostRecentContentTimeInMs );

         if (periodTimeInMs > mostRecentContentTimeInMs) {
             representationSeekCtx->isAtEof = true;
            errCode = B_ERROR_INVALID_PARAMETER;    /* Can't seek past most recent time. */
            goto seek_failed;
         }

         PRINTMSG_SEEK(("%s:%d: periodTimeInMs=%"PRIu64"", myFuncName, __LINE__, periodTimeInMs));
         PRINTMSG_SEEK(("%s:%d: mostRecentContentTimeInMs=%"PRIu64" Ms from most recent=%"PRIu64"", myFuncName, __LINE__,
                                               mostRecentContentTimeInMs, mostRecentContentTimeInMs - periodTimeInMs ));

    }
    else {  /* GARYWASHERE: Maybe this shouldn't be an "else"... maybe we should do this check for dynamic MPDs also. */
        BDBG_NUL(("%s:%d: periodTimeInMs=%"PRIu64"   periodDurationInMs=%"PRIu64"", myFuncName, __LINE__, periodTimeInMs, representationInfo->parentObj->parentObj->periodDurationInMs));
        if (representationInfo->parentObj->parentObj->periodDurationInMs &&
            periodTimeInMs >= representationInfo->parentObj->parentObj->periodDurationInMs) {
            representationSeekCtx->isAtEof = true;
            errCode = B_ERROR_INVALID_PARAMETER;    /* Probably just means end-of-period. */
            goto seek_failed;
        }
    }

    /*------------------------------------------------------------
     * A Representation's segments can be specified by either 1) a
     * SegmentBase, 2) a SegmentList, or 3) a SegmentTemplate.
     * Handle each of these cases separately.
     *----------------------------------------------------------*/
    segmentBase     = MPEG_DASH_GET_INHERITED_SEGMENT_TYPE( representationInfo, segmentBase);
    segmentList     = MPEG_DASH_GET_INHERITED_SEGMENT_TYPE( representationInfo, segmentList);
    segmentTemplate = MPEG_DASH_GET_INHERITED_SEGMENT_TYPE( representationInfo, segmentTemplate);

    representationSeekCtx->segmentBase      = NULL;
    representationSeekCtx->segmentList      = NULL;
    representationSeekCtx->segmentTemplate  = NULL;

    if (segmentBase) {
        PRINTMSG_SEEK(("%s:%d : Representation id: %s has SegmentBase", myFuncName, __LINE__ , representationInfo->id   ));

        BDBG_ERR(("SegmentBase Representations are not yet supported!"));
        /* GARYWASHERE: Add support for SegmentBase Representations. */

        errCode = BERR_TRACE(B_ERROR_NOT_SUPPORTED);
        goto seek_failed;
    }
    else if (segmentList) {
        MpegDashSegmentUrlInfo   * segmentUrl;
        unsigned long startNumber              = representationInfo->inheritedStartNumber;
        unsigned long thisSegmentNumber;            /* zero-based */
        unsigned long durationInTimescaleUnits = representationInfo->inheritedDuration;
        unsigned long targetSegmentNumber = 0;      /* zero-based */
        unsigned long segmentPeriodTimeInTimescaleUnits = 0;
        unsigned long segmentTimelineTime = 0;
        unsigned long segmentDurationInTimescaleUnits = 0;

        PRINTMSG_SEEK(("%s:%d : Representation id: %s has SegmentList", myFuncName, __LINE__  , representationInfo->id   ));

        /*------------------------------------------------------------
         * First, we need to map our PeriodTime into a SegmentNumber.
         * We do that differently depending on whether we have a
         * durationInTimescaleUnits (constant segment durations), or a
         * segmentTimeline (varying segment durations).
         *----------------------------------------------------------*/
        /* For SegmentLists, the our segmentNumber is just the
         * zero-based index into the list.  According to the spec, the
         * segmentNumber starts with startNumber, but that just makes
         * everything confusing.  */
        if (durationInTimescaleUnits) {
            targetSegmentNumber                 = periodTimeInTimescaleUnits / durationInTimescaleUnits;
            segmentPeriodTimeInTimescaleUnits   = targetSegmentNumber * durationInTimescaleUnits;   /* This will round down to a segment boundary. */
            segmentDurationInTimescaleUnits     = durationInTimescaleUnits;
            PRINTMSG_SEEK(("%s:%d : Using fixed duration from Representation, durationInTsUnits: %lu", myFuncName, __LINE__, segmentDurationInTimescaleUnits ));
        }

        else if ( segmentTemplate->baseTypeMultipleSegmentBase->segmentTimeline ) {
            MpegDashSegmentTimelineElemInfo   resultSegmentTimelineElem;

            B_PlaybackIp_MpegDashSegmentTimelineLookup(
                                        playback_ip,
                                        segmentList->baseTypeMultipleSegmentBase->segmentTimeline,
                                        periodTimeInTimescaleUnits,
                                        &resultSegmentTimelineElem); /* put the result here */

            if (!resultSegmentTimelineElem.parsed)
            {
                BDBG_ERR(("%s: Can't find time in SegmentTimeline!", myFuncName));
                errCode = BERR_TRACE(B_ERROR_UNKNOWN);
                goto seek_failed;
            }

            targetSegmentNumber                 = resultSegmentTimelineElem.segmentNumber + startNumber;
            segmentPeriodTimeInTimescaleUnits   = resultSegmentTimelineElem.segmentTimeInTimescaleUnits;
            segmentDurationInTimescaleUnits     = resultSegmentTimelineElem.cooked.d;
            segmentTimelineTime                 = resultSegmentTimelineElem.segmentEptInTimescaleUnits;
            PRINTMSG_SEEK(("%s:%d : Using duration from segmentTimeline, durationInTsUnits: %lu", myFuncName, __LINE__, segmentDurationInTimescaleUnits ));
        }

        else {
            /* Okay, so if we don't have either a duration or a segmentTimeline, then there should be only
               one segment, and that should be "startNumber". */
            targetSegmentNumber = 0;
            segmentPeriodTimeInTimescaleUnits = 0;

            /* Since there's only one segment in the Period, use the
             * Period's duration as the Segment's duration.  But convert
             * to timescale units first.*/
            segmentDurationInTimescaleUnits = (representationInfo->parentObj->parentObj->periodDurationInMs * timescale) / 1000;
            PRINTMSG_SEEK(("%s:%d : Only one segment in Period, using Period duration, durationInTsUnits: %lu", myFuncName, __LINE__, segmentDurationInTimescaleUnits ));
        }

        /* Okay, we have the segment number, now find that Segment in
         * the SegmentList. */

        PRINTMSG_SEEK(("%s:%d : Looking in SegmentList for segment number %lu", myFuncName, __LINE__, targetSegmentNumber    ));

        for (segmentUrl = BLST_Q_FIRST(&segmentList->mpegDashSegmentUrlInfoHead) , thisSegmentNumber = 0 ;
             segmentUrl && thisSegmentNumber < targetSegmentNumber ;
             segmentUrl = BLST_Q_NEXT(segmentUrl, nextSegmentUrl) , thisSegmentNumber++ )
        {
            PRINTMSG_SEEK(("%s:%d : num: %lu Found segmentUrl: %p  %s ", myFuncName, __LINE__, thisSegmentNumber, (void *)segmentUrl, segmentUrl->media  ));
        }
        PRINTMSG_SEEK(("%s:%d : Out of loop.  thisSegmentNumber: %lu URL: %s ", myFuncName, __LINE__, thisSegmentNumber, segmentUrl?segmentUrl->media:"nil"  ));

        if(!segmentUrl) {
            BDBG_ERR(("Can't find segmentNumber %lu in SegmentList with only %lu segments", targetSegmentNumber, thisSegmentNumber ));
            errCode = BERR_TRACE(B_ERROR_UNKNOWN);  /* GARYWASHERE: Maybe this should be handled as EOF??? */
            goto seek_failed;
        }

        representationSeekCtx->segmentUrlInfo               = segmentUrl;
        representationSeekCtx->segmentNumber                = targetSegmentNumber + startNumber;
        representationSeekCtx->timescale                    = timescale;
        representationSeekCtx->periodTimeInTimescaleUnits   = segmentPeriodTimeInTimescaleUnits;
        representationSeekCtx->durationInTimescaleUnits     = segmentDurationInTimescaleUnits;
        representationSeekCtx->segmentTimelineTime          = segmentTimelineTime;
        representationSeekCtx->segmentList                  = segmentList;

        PRINTMSG_SEEK(("%s:%d : SegmentDurationInMs: %llu", myFuncName, __LINE__, ((unsigned long long)segmentDurationInTimescaleUnits*1000)/timescale ));
    }
    else if (segmentTemplate) {
        unsigned long startNumber              = representationInfo->inheritedStartNumber;
        unsigned long durationInTimescaleUnits = representationInfo->inheritedDuration;
        unsigned long targetSegmentNumber = 0;
        uint64_t      segmentPeriodTimeInTimescaleUnits = 0;
        unsigned long segmentTimelineTime = 0;
        unsigned long segmentDurationInTimescaleUnits = 0;

        PRINTMSG_SEEK(("%s:%d : Representation id: %s has SegmentTemplate", myFuncName, __LINE__  , representationInfo->id   ));

        /*------------------------------------------------------------
         * First, we need to map our PeriodTime into a SegmentNumber.
         * We do that differently depending on whether we have a
         * durationInTimescaleUnits (constant segment durations), or a
         * segmentTimeline (varying segment durations).
         *
         * (Ref:ISO/IEC 23009-1:2012 Sec 5.3.9.2.1)   If there is a
         * SegmentTemplate, then the segment durations are indicated by
         * either a "duration" attribute, or a SegmentTimeline element
         * under the SegmentTemplate.
         *----------------------------------------------------------*/
        if (durationInTimescaleUnits) {
            targetSegmentNumber                 = periodTimeInTimescaleUnits / durationInTimescaleUnits;
            segmentPeriodTimeInTimescaleUnits   = (uint64_t)targetSegmentNumber * durationInTimescaleUnits;   /* This will round down to a segment boundary. */
            segmentDurationInTimescaleUnits     = durationInTimescaleUnits;

            PRINTMSG_SEEK(("%s:%d: durationInTimescaleUnits=%lu", myFuncName, __LINE__, durationInTimescaleUnits));
            PRINTMSG_SEEK(("%s:%d: targetSegmentNumber=%lu", myFuncName, __LINE__, targetSegmentNumber));
            PRINTMSG_SEEK(("%s:%d: segmentPeriodTimeInTimescaleUnits=%"PRIu64"", myFuncName, __LINE__, segmentPeriodTimeInTimescaleUnits));
            PRINTMSG_SEEK(("%s:%d: segmentDurationInTimescaleUnits=%lu", myFuncName, __LINE__, segmentDurationInTimescaleUnits));
            PRINTMSG_SEEK(("%s:%d : Using fixed duration from Representation, durationInTsUnits: %lu", myFuncName, __LINE__, segmentDurationInTimescaleUnits ));
        }
        else if ( segmentTemplate->baseTypeMultipleSegmentBase->segmentTimeline ) {
            MpegDashSegmentTimelineElemInfo   resultSegmentTimelineElem;

            B_PlaybackIp_MpegDashSegmentTimelineLookup(
                                        playback_ip,
                                        segmentTemplate->baseTypeMultipleSegmentBase->segmentTimeline,
                                        periodTimeInTimescaleUnits,
                                        &resultSegmentTimelineElem); /* put the result here */

            if (!resultSegmentTimelineElem.parsed)
            {
                BDBG_ERR(("%s: Can't find time in SegmentTimeline!", myFuncName));
                errCode = BERR_TRACE(B_ERROR_UNKNOWN);
                goto seek_failed;
            }

            targetSegmentNumber                 = resultSegmentTimelineElem.segmentNumber;
            segmentPeriodTimeInTimescaleUnits   = resultSegmentTimelineElem.segmentTimeInTimescaleUnits;
            segmentDurationInTimescaleUnits     = resultSegmentTimelineElem.cooked.d;
            segmentTimelineTime                 = resultSegmentTimelineElem.segmentEptInTimescaleUnits;
            PRINTMSG_SEEK(("%s:%d : Using duration from segmentTimeline, durationInTsUnits: %lu", myFuncName, __LINE__, segmentDurationInTimescaleUnits ));
        }
        else {
            /* Okay, so if we don't have either a duration or a segmentTimeline, then there should be only
               one segment, and that should be "startNumber". */
            targetSegmentNumber = 0;    /* Call it zero for now... we'll add startNumber later.     */
            segmentPeriodTimeInTimescaleUnits = 0;

            /* Since there's only one segment in the Period, use the
             * Period's duration as the Segment's duration.  But convert
             * to timescale units first.*/
            segmentDurationInTimescaleUnits = (representationInfo->parentObj->parentObj->periodDurationInMs * timescale) / 1000;
            PRINTMSG_SEEK(("%s:%d : Only one segment in Period, using Period duration, durationInTsUnits: %lu", myFuncName, __LINE__, segmentDurationInTimescaleUnits ));
        }

        representationSeekCtx->segmentTemplate              = segmentTemplate;
        representationSeekCtx->segmentNumber                = targetSegmentNumber + startNumber;    /* Now startNumber-based */
        representationSeekCtx->timescale                    = timescale;
        representationSeekCtx->periodTimeInTimescaleUnits   = segmentPeriodTimeInTimescaleUnits;
        representationSeekCtx->durationInTimescaleUnits     = segmentDurationInTimescaleUnits;
        representationSeekCtx->segmentTimelineTime          = segmentTimelineTime;

        PRINTMSG_SEEK(("%s:%d : SegmentDurationInMs: %llu", myFuncName, __LINE__, ((unsigned long long)segmentDurationInTimescaleUnits*1000)/timescale ));
    }
    else
    {
        BDBG_ERR(("%s: Representation has no segmentBase, segmentList, or segmentTemplate!", myFuncName));
        errCode = BERR_TRACE(B_ERROR_UNKNOWN);
        goto seek_failed;
    }

    /* Fill in the remaining fields of the seek context structure. */
    representationSeekCtx->periodTimeInMs       = representationSeekCtx->periodTimeInTimescaleUnits * 1000 / timescale;
    representationSeekCtx->representationInfo   = representationInfo;
    representationSeekCtx->representationId     = representationInfo->id;

    representationSeekCtx->isAtEof = false;

    PRINTMSG_SEEK(("%s:%d : Seek done. PeriodTimeInMs:%"PRIu64" PeriodTimeInTs:%"PRIu64" Timescale:%lu SegTimelineTime:%lu SegNum:%u",
                    myFuncName, __LINE__  ,
                   representationSeekCtx->periodTimeInMs,
                   representationSeekCtx->periodTimeInTimescaleUnits,
                   representationSeekCtx->timescale,
                   representationSeekCtx->segmentTimelineTime,
                   representationSeekCtx->segmentNumber));

    return B_ERROR_SUCCESS;

seek_failed:
    return errCode;
}


/*****************************************************************************
 * Given a Representation and PeriodTime (specified in the milliseconds,
 * update a RepresentationSeekContext with information needed to access
 * the segments that correspond to that time.
 *****************************************************************************/
static B_PlaybackIpError
B_PlaybackIp_MpegDashSeekInRepresentationByMs(B_PlaybackIpHandle playback_ip, MpegDashRepresentationInfo *representationInfo,  MpegDashRepresentationSeekContext *representationSeekCtx, uint64_t periodTimeInMs)
{
    B_PlaybackIpError    errCode = B_ERROR_UNKNOWN;
    char               * myFuncName = "SeekInReprByMs";

    BSTD_UNUSED(playback_ip);

    unsigned long   timescale = representationInfo->inheritedTimescale;
    uint64_t        periodTimeInTimescaleUnits = (periodTimeInMs * timescale ) / 1000;

    PRINTMSG_SEEK(("%s:%d: Seeking to periodTimeInTsUnits: %"PRIu64" Timescale: %lu", myFuncName, __LINE__, periodTimeInTimescaleUnits, timescale));

    errCode = B_PlaybackIp_MpegDashSeekInRepresentationByTimescale(playback_ip, representationInfo,  representationSeekCtx, periodTimeInTimescaleUnits);
    if (errCode) {
        BDBG_ERR(("%s: Can't find seek time in Representation by Ms", myFuncName));
        errCode = BERR_TRACE(errCode);
        goto seek_failed;
    }
    return B_ERROR_SUCCESS;

seek_failed:
    return errCode;
}


/*****************************************************************************
 * Assign the Nexus playpump handles to be used for audio and
 * video.
 *****************************************************************************/
void B_PlaybackIp_MpegDashSetPlaypumpHandles(
    B_PlaybackIpHandle playback_ip,
    MpegDashAdaptationSetInfo   * audioAdaptationSet,
    MpegDashAdaptationSetInfo   * videoAdaptationSet
    )
{
    char * myFuncName = "SetPlaypumpHandles";

    BDBG_ASSERT(playback_ip);

    /* Use the primary playpump for video. */
    if (videoAdaptationSet) {
        playback_ip->mpegDashSessionState->pVideoPlaypump = &playback_ip->nexusHandles.playpump;
    }

    if (audioAdaptationSet) {
        /* If audio and video are in the same AdaptationSet, then
         * audio has to use the primary playpump also. */
        if (audioAdaptationSet == videoAdaptationSet) {
            playback_ip->mpegDashSessionState->pAudioPlaypump = &playback_ip->nexusHandles.playpump;
        }
        else
        {
            playback_ip->mpegDashSessionState->pAudioPlaypump = &playback_ip->nexusHandles.playpump2;

        }
    }
    PRINTMSG_SEEK(("%s:%d: Using NEXUS_PlaypumpHandles: Audio:%s  Video:%s",
               myFuncName, __LINE__,
               playback_ip->mpegDashSessionState->pAudioPlaypump==&playback_ip->nexusHandles.playpump  ? "playpump"  :
               playback_ip->mpegDashSessionState->pAudioPlaypump==&playback_ip->nexusHandles.playpump2 ? "playpump2" :
                                                                                                         " <none>",
               playback_ip->mpegDashSessionState->pVideoPlaypump==&playback_ip->nexusHandles.playpump  ? "playpump"  :
               playback_ip->mpegDashSessionState->pVideoPlaypump==&playback_ip->nexusHandles.playpump2 ? "playpump2" :
                                                                                                         " <none>"));

    return;
}


/*****************************************************************************
 * Select the proper playpump handle depending on the type of content
 * (audio or video).
 *****************************************************************************/
NEXUS_PlaypumpHandle B_PlaybackIp_MpegDashGetPlaypumpHandle(
    B_PlaybackIpHandle playback_ip,
    NEXUS_PidType      pidType
    )
{
    NEXUS_PlaypumpHandle  *pPlaypumpHandle;

    BDBG_ASSERT(playback_ip);

    if (pidType == NEXUS_PidType_eVideo)
    {
        pPlaypumpHandle = playback_ip->mpegDashSessionState->pVideoPlaypump;
    }
    else if (pidType == NEXUS_PidType_eAudio)
    {
        pPlaypumpHandle = playback_ip->mpegDashSessionState->pAudioPlaypump;
    }
    else
    {
        BDBG_ERR(("%s: Invalid PidType=%d, Can't determine playpump handle!", __FUNCTION__, pidType));
        return NULL;
    }

    if (pPlaypumpHandle) {
        return *pPlaypumpHandle;
    }

    return NULL;
}


/*****************************************************************************
 *  Check to see if the specified pid has an opened pid channel.  If not,
 *  then Open a new slave PidChannel, and add it to the list of slave pids.
 *****************************************************************************/
static bool
B_PlaybackIp_MpegDashRemoveSlavePidChannel(B_PlaybackIpHandle playback_ip, NEXUS_PidType pidType, MpegDashSlavePidEntry  * slavePidEntry)
{
    char                          * myFuncName = "RemoveSlavePidChan";
    MpegDashSessionState          * mpegDashSession = playback_ip->mpegDashSessionState;
    struct mpegDashSlavePidHead   * slavePidHead;
    char                          * pidTypeText;
    NEXUS_PidChannelHandle          masterPidChannel;
    NEXUS_Error                     errCode;
    NEXUS_PlaypumpHandle            myPlaypump = NULL;

    /* Set some common variables based on the type (audio/video) of pid. */
    if (pidType == NEXUS_PidType_eAudio) {
        pidTypeText  = "Audio";
        slavePidHead = &mpegDashSession->audioSlavePidHead;
        masterPidChannel   = playback_ip->nexusHandles.audioPidChannel;
    }
    else if (pidType == NEXUS_PidType_eVideo) {
        pidTypeText  = "Video";
        slavePidHead = &mpegDashSession->videoSlavePidHead;
        masterPidChannel   = playback_ip->nexusHandles.videoPidChannel;
    }
    else {
        BDBG_ERR(("%s: Unable to determine media type of PidChannel!", myFuncName));
        goto error;
    }

    myPlaypump = B_PlaybackIp_MpegDashGetPlaypumpHandle(playback_ip, pidType);

    PRINTMSG_SEEK(("%s:%d: %s Pid: Removing slave pid %d (handle %p) from %s Master pid %p",
               myFuncName, __LINE__, pidTypeText,
               slavePidEntry->pid, (void *)slavePidEntry->pidChannelHandle, pidTypeText, (void *)masterPidChannel));

    NEXUS_PidChannel_RemoveSlavePidChannel( masterPidChannel, slavePidEntry->pidChannelHandle);

    errCode = NEXUS_Playpump_ClosePidChannel(myPlaypump, slavePidEntry->pidChannelHandle);
    if (errCode) { errCode = BERR_TRACE(errCode) ; goto error; }

    BLST_Q_REMOVE(slavePidHead, slavePidEntry, nextSlavePidEntry);
    BKNI_Free(slavePidEntry);

    return true;    /* Normal Success */

error:
    return false;
}


/*****************************************************************************
 *  Check to see if the specified pid has an opened pid channel.  If not,
 *  then Open a new slave PidChannel, and add it to the list of slave pids.
 *****************************************************************************/
static bool
B_PlaybackIp_MpegDashAddSlavePidChannel(B_PlaybackIpHandle playback_ip, NEXUS_PidType pidType, unsigned newPid)
{
    char                          * myFuncName = "AddSlavePidChan";
    MpegDashSessionState          * mpegDashSession = playback_ip->mpegDashSessionState;
    struct mpegDashSlavePidHead   * slavePidHead;
    MpegDashSlavePidEntry         * slavePidEntry;
    char                          * pidTypeText;

    NEXUS_PidChannelHandle          masterPidChannel;
    uint16_t                        masterPid;
    NEXUS_PlaypumpOpenPidChannelSettings  pidChannelSettings;
    NEXUS_Error                     errCode;
    bool                            needToAddPid = true;

    /* Set some common variables based on the type (audio/video) of pid. */
    if (pidType == NEXUS_PidType_eAudio) {
        pidTypeText  = "Audio";
        slavePidHead = &mpegDashSession->audioSlavePidHead;
        masterPid    = mpegDashSession->audioMasterPid;
        masterPidChannel   = playback_ip->nexusHandles.audioPidChannel;
    }
    else if (pidType == NEXUS_PidType_eVideo) {
        pidTypeText  = "Video";
        slavePidHead = &mpegDashSession->videoSlavePidHead;
        masterPid    = mpegDashSession->videoMasterPid;
        masterPidChannel   = playback_ip->nexusHandles.videoPidChannel;
    }
    else {
        BDBG_ERR(("%s: Unable to determine media type of PidChannel!",myFuncName));
        goto error;
    }

    PRINTMSG_SEEK(("%s:%d: Checking for existing PidChannel for %s Pid: %u", myFuncName, __LINE__, pidTypeText,  newPid));

    if (newPid == masterPid) {
        /* If the new pid is the master pid, then we it already has a pid channel that we can use. */
        PRINTMSG_SEEK(("%s:%d: %s Pid: %u already exists as %s Master pid", myFuncName, __LINE__, pidTypeText,  newPid, pidTypeText));
        needToAddPid = false;
    }
    else {
        /* Otherwise, look through the slave pid list to see if we've already created a pid channel for this pid. */

        for(slavePidEntry = BLST_Q_FIRST(slavePidHead) ; slavePidEntry ; slavePidEntry = BLST_Q_NEXT(slavePidEntry, nextSlavePidEntry) )
        {
            if (slavePidEntry->pid == newPid) {
                PRINTMSG_SEEK(("%s:%d: %s Pid: %u (%p) already exists as a %s Slave pid", myFuncName, __LINE__, pidTypeText,  newPid, (void *)slavePidEntry->pidChannelHandle, pidTypeText));
                needToAddPid = false;       /* Found the new pid in the list... don't need to add it.*/
                break;
            }
        }
    }

    /* Open the new pid channel if we need to. */
    if (needToAddPid)
    {
        NEXUS_PidChannelHandle   slavePidChannel;
        MpegDashSlavePidEntry  * slavePidEntry;
        NEXUS_PlaypumpHandle     myPlaypump = B_PlaybackIp_MpegDashGetPlaypumpHandle(playback_ip,pidType);

        PRINTMSG_SEEK(("%s:%d: Pid: %u doesn't exist, need to create...", myFuncName, __LINE__, newPid));

        /* Open the pid channel */
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = pidType;
        if (pidType == NEXUS_PidType_eAudio) {
            pidChannelSettings.pidTypeSettings.audio.codec = playback_ip->psi.audioCodec;
        }
        slavePidChannel = NEXUS_Playpump_OpenPidChannel(myPlaypump, newPid, &pidChannelSettings);
        if (!slavePidChannel) {BDBG_ERR(("NEXUS_Playpump_OpenPidChannel() failed  at %s:%d, Exiting...\n", myFuncName, __LINE__)); goto error;}

        /* And then slave it off of the master pid channel.  This will pass streams that match
         * either the master pid, or any of the slave pids through to the decoder.  This allows
         * for a clean transition when the stream changes from one pid to the next.  */

        PRINTMSG_SEEK(("%s:%d: %s Pid: Adding pid %u (%p) as slave to %p", myFuncName, __LINE__, pidTypeText,  newPid, (void *)slavePidChannel, (void *)masterPidChannel));

        errCode = NEXUS_PidChannel_AddSlavePidChannel(masterPidChannel, slavePidChannel, NULL);
        if (errCode) {
            BERR_TRACE(errCode); goto error;
        }

        /* Now, we need to keep track of the slave pid channels that we are creating,
         * so we can clean them up when we need to.  Also so we can see when we already
         * have a pid channel for a pid.  Add the new pid/pid channel into the list
         * of slave pids.  */
        slavePidEntry = BKNI_Malloc(sizeof(*slavePidEntry));
        if (NULL == slavePidEntry ) {
            (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error;
        }
        slavePidEntry->pidChannelHandle = slavePidChannel;
        slavePidEntry->pid              = newPid;
        BLST_Q_INSERT_TAIL(slavePidHead, slavePidEntry, nextSlavePidEntry);                                                                   \

        PRINTMSG_SEEK(("%s:%d: %s Pid: %u added and slaved successfully.", myFuncName, __LINE__, pidTypeText,  newPid));
    }

    return true;    /* Normal Success */

error:
    return false;
}


/*****************************************************************************
 * Given a SeekContext, check to see if we need to change to a different
 * Representation (mainly because of changes to the measured bandwidth).
 * If the Representation(s) should be changed, then go ahead and do it.
 *****************************************************************************/
static bool
B_PlaybackIp_MpegDashCheckForRepresentationChange(B_PlaybackIpHandle playback_ip, MpegDashSeekContext * requestSeekCtx)
{
    char                               * myFuncName = "CheckForReprChange";
    MpegDashSessionState               * mpegDashSession = playback_ip->mpegDashSessionState;
    MpegDashRepresentationSeekContext  * representationSeekCtx;
    MpegDashAdaptationSetInfo          * audioAdaptationSet = NULL;
    MpegDashAdaptationSetInfo          * videoAdaptationSet = NULL;
    MpegDashRepresentationInfo         * audioRepresentationInfo = NULL;
    MpegDashRepresentationInfo         * videoRepresentationInfo = NULL;
    MpegDashRepresentationInfo         * newAudioRepresentationInfo = NULL;
    MpegDashRepresentationInfo         * newVideoRepresentationInfo = NULL;
    unsigned                             availableBandwidthInKbps;
    unsigned long                        newTimeInTimescaleUnits;

    /* Let's check to see if we need to change to a different Representation
     * based on the current measured network bandwidth.*/
    availableBandwidthInKbps = B_PlaybackIp_MpegDashGetCurrentBandwidth(&mpegDashSession->bandwidthContext);

    /* Get the AdaptationSet (parent) for each of the current Representations.*/
    audioRepresentationInfo = requestSeekCtx->audio.representationInfo;
    if (audioRepresentationInfo) {
        audioAdaptationSet = audioRepresentationInfo->parentObj;
    }

    videoRepresentationInfo = requestSeekCtx->video.representationInfo;
    if (videoRepresentationInfo) {
        videoAdaptationSet = videoRepresentationInfo->parentObj;
    }

    /* Now choose the best representations from those AdaptationSets. */
    if (!B_PlaybackIp_MpegDashSelectRepresentations(playback_ip,
                                                    audioAdaptationSet, videoAdaptationSet,
                                                    &newAudioRepresentationInfo, &newVideoRepresentationInfo,
                                                    availableBandwidthInKbps) ) {
        BDBG_ERR(("%s: Can't find any usable Audio Representations", myFuncName));
        goto seekError;
    }

    /* Add some hysteresis here... switch to a lower bandwidth as
     * soon as the available bandwith crosses the sum of the
     * Representations' bandwith threshhold.  But don't switch to a
     * higher bandwidth unless the total expected (A+V) bandwidth is
     * above the threshhold by a certain percentage.  */
    if (audioRepresentationInfo != newAudioRepresentationInfo ||
        videoRepresentationInfo != newVideoRepresentationInfo)
    {
        unsigned oldBandwidthInKbps = 0;
        unsigned newBandwidthInKbps = 0;

        if (videoRepresentationInfo) oldBandwidthInKbps += videoRepresentationInfo->bandwidthNumeric;
        if (audioRepresentationInfo  && audioRepresentationInfo != videoRepresentationInfo) oldBandwidthInKbps += audioRepresentationInfo->bandwidthNumeric;

        if (newVideoRepresentationInfo) newBandwidthInKbps += newVideoRepresentationInfo->bandwidthNumeric;
        if (newAudioRepresentationInfo && newAudioRepresentationInfo != newVideoRepresentationInfo) newBandwidthInKbps += newAudioRepresentationInfo->bandwidthNumeric;

        oldBandwidthInKbps /= 1000;     /* Convert from bps to Kbps. */
        newBandwidthInKbps /= 1000;

        BDBG_NUL(("%s:%d: oldBandwidthInKbps=      %u Kbps", myFuncName, __LINE__, oldBandwidthInKbps));
        BDBG_NUL(("%s:%d: newBandwidthInKbps=      %u Kbps", myFuncName, __LINE__, newBandwidthInKbps));
        BDBG_NUL(("%s:%d: availableBandwidthInKbps=%u Kbps", myFuncName, __LINE__, availableBandwidthInKbps));

        if (newBandwidthInKbps > oldBandwidthInKbps)
        {
            if (availableBandwidthInKbps < (newBandwidthInKbps * 12) / 10)  /* Exceed 120% of required bandwidth? */
            {
                newAudioRepresentationInfo = audioRepresentationInfo;   /* Don't change to new Repr */
                newVideoRepresentationInfo = videoRepresentationInfo;
                BDBG_NUL(("%s:%d : Cancelling Representation change!!!  ", myFuncName, __LINE__   ));
            }
        }
    }

    /* Now see if we chose a different video Representation than the one we're using now.
     * If so, then we'll need to change to it. */
    if (newVideoRepresentationInfo != videoRepresentationInfo &&
        newVideoRepresentationInfo != NULL) {
        unsigned long   newTimeInNewTimescaleUnits;

        PRINTMSG_SEEK(("%s:%d: Changing video Representation from %s to %s.  Seeking and probing...", myFuncName, __LINE__,
                       videoRepresentationInfo->id, newVideoRepresentationInfo->id));

        representationSeekCtx = &requestSeekCtx->video;

        newTimeInTimescaleUnits = newTimeInNewTimescaleUnits = representationSeekCtx->periodTimeInTimescaleUnits;

        /* If the new Representation has a different timescale, change our
         * seek time into the new timescale units.*/
        if (newVideoRepresentationInfo->inheritedTimescale != videoRepresentationInfo->inheritedTimescale) {
            newTimeInNewTimescaleUnits = (newTimeInTimescaleUnits * newVideoRepresentationInfo->inheritedTimescale) / videoRepresentationInfo->inheritedTimescale;
        }

        /* Seek into the new Representation. */
        if (B_PlaybackIp_MpegDashSeekInRepresentationByTimescale(playback_ip, newVideoRepresentationInfo , representationSeekCtx, newTimeInNewTimescaleUnits)) {
            BDBG_ERR(("%s: Can't seek to next segment in Representation %s", myFuncName, newVideoRepresentationInfo->id));
            goto seekError;
        }

        /* That's all for now. Content probing and slave pid handling
         * is done by the download thread. */

    }  /* End if switching video Representations. */


    /* Now see if we chose a different audio Representation than the one we're using now.
     * If so, then we'll need to change to the new Representation now. */
    if (newAudioRepresentationInfo != audioRepresentationInfo &&
        newAudioRepresentationInfo != NULL &&
        newAudioRepresentationInfo != newVideoRepresentationInfo) {
        unsigned long   newTimeInNewTimescaleUnits;

        PRINTMSG_SEEK(("%s:%d: Changing audio Representation from %s to %s.  Seeking and probing...", myFuncName, __LINE__,
                       audioRepresentationInfo->id, newAudioRepresentationInfo->id));

        representationSeekCtx = &requestSeekCtx->audio;

        newTimeInTimescaleUnits = newTimeInNewTimescaleUnits = representationSeekCtx->periodTimeInTimescaleUnits;

        /* If the new Representation has a different timescale, change our
         * seek time into the new timescale units.*/
        if (newAudioRepresentationInfo->inheritedTimescale != audioRepresentationInfo->inheritedTimescale) {
            newTimeInNewTimescaleUnits = (newTimeInTimescaleUnits * newAudioRepresentationInfo->inheritedTimescale) / audioRepresentationInfo->inheritedTimescale;
        }

        /* Seek into the new Representation. */
        if (B_PlaybackIp_MpegDashSeekInRepresentationByTimescale(playback_ip, newAudioRepresentationInfo , representationSeekCtx, newTimeInNewTimescaleUnits)) {
            BDBG_ERR(("%s: Can't seek to next segment in Representation %s", myFuncName, newAudioRepresentationInfo->id));
            goto seekError;
        }

        /* That's all for now. Content probing and slave pid handling
         * is done by the download thread. */

    }  /* End if switching audio Representations. */


    /* GARYWASHERE: Need to update seek time in requestSeekCtx... */
    /* requestSeekCtx->periodTime = ????? */


    PRINTMSG_SEEK(("%s:%d: Done checking for Representation change.", myFuncName, __LINE__));
    return true;

seekError:
    return false;
}


static B_PlaybackIpError
B_PlaybackIp_MpegDashSeekSwitchAdaptationSet(B_PlaybackIpHandle playback_ip, MpegDashSeekContext * requestSeekCtx)
{
    char                               * myFuncName = "SwitchAdaptationSet";
    MpegDashRepresentationInfo         * representationInfo;
    MpegDashRepresentationSeekContext  * representationSeekCtx;

    BSTD_UNUSED(playback_ip);

    representationSeekCtx = requestSeekCtx->currentRepresentationSeekContext;
    representationInfo    = representationSeekCtx->representationInfo;

    PRINTMSG_SEEK(("%s:%d: Entry: Seek is at Repr:%p id:%s timeInTs:%"PRIu64" timeInMs:%"PRIu64" status:%s",
                   myFuncName, __LINE__,
                   (void *)representationSeekCtx->representationInfo,
                   representationSeekCtx->representationId,
                   representationSeekCtx->periodTimeInTimescaleUnits,
                   (representationSeekCtx->periodTimeInTimescaleUnits*(uint64_t)1000)/representationSeekCtx->timescale,
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eInvalid    ? "Invalid"   :
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eValid      ? "Valid"     :
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eDownloaded ? "Downloaded":
                                                                                               "<Unknown>"  ));
    /* Mark the current representation's seek ctx as Downloaded. */
    representationSeekCtx->status = B_PlaybackMpegDashSeekStatus_eDownloaded;

    /* If we have both audio and video representations, and they're not the same one, then
     * we'll need to toggle between the two.  If we only have one Representation, then*
     * there's no need to toggle. */
    if (requestSeekCtx->audio.representationInfo  &&
        requestSeekCtx->video.representationInfo  &&
        requestSeekCtx->audio.representationInfo != requestSeekCtx->video.representationInfo)
    {
        /* GARYWASHERE: Be sure to handle and test both audio and video in the
         * same representation!!!*/
        if (representationSeekCtx  == &requestSeekCtx->video) {
            if (requestSeekCtx->audio.representationInfo)
            {
                PRINTMSG_SEEK(("%s:%d : Switching seek's Repr: video->audio ", myFuncName, __LINE__   ));
                representationSeekCtx = &requestSeekCtx->audio;
            }
        }
        else {
            if (requestSeekCtx->video.representationInfo)
            {
                PRINTMSG_SEEK(("%s:%d : Switching seek's Repr: audio->video", myFuncName, __LINE__   ));
                representationSeekCtx  = &requestSeekCtx->video;
            }
        }
    }

    requestSeekCtx->currentRepresentationSeekContext = representationSeekCtx;

    PRINTMSG_SEEK(("%s:%d: Exit: Seek is at Repr:%p id:%s timeInTs:%"PRIu64" timeInMs:%"PRIu64" status:%s",
                   myFuncName, __LINE__,
                   (void *)representationSeekCtx->representationInfo,
                   representationSeekCtx->representationId,
                   representationSeekCtx->periodTimeInTimescaleUnits,
                   (representationSeekCtx->periodTimeInTimescaleUnits*(uint64_t)1000)/representationSeekCtx->timescale,
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eInvalid    ? "Invalid"   :
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eValid      ? "Valid"     :
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eDownloaded ? "Downloaded":
                                                                                               "<Unknown>"  ));
    return B_ERROR_SUCCESS;
}

/*****************************************************************************
 * Given a SeekContext, advance past the current segment.  This involves
 * adding the duration of the current Representation's current segment,
 * then possibly switching from the current Representation to the alternate
 * (audio/video) Representation (for the case where audio and video are
 * in different Representations).
 *****************************************************************************/
static B_PlaybackIpError
B_PlaybackIp_MpegDashSeekAdvanceBySegment(B_PlaybackIpHandle playback_ip, MpegDashSeekContext * requestSeekCtx)
{
    char                               * myFuncName = "AdvanceBySegment";
    MpegDashRepresentationInfo         * representationInfo;
    MpegDashRepresentationSeekContext  * representationSeekCtx;
    B_PlaybackIpError                    errCode = B_ERROR_SUCCESS;

    uint64_t   newTimeInTimescaleUnits;


    representationSeekCtx = requestSeekCtx->currentRepresentationSeekContext;
    representationInfo    = representationSeekCtx->representationInfo;

    PRINTMSG_SEEK(("%s:%d: Entry: Seek is at Repr:%p id:%s timeInTs:%"PRIu64" status:%s",
                   myFuncName, __LINE__,
                   (void *)representationSeekCtx->representationInfo,
                   representationSeekCtx->representationId,
                   representationSeekCtx->periodTimeInTimescaleUnits,
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eInvalid    ? "Invalid"   :
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eValid      ? "Valid"     :
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eDownloaded ? "Downloaded":
                                                                                               "<Unknown>"  ));
    if (representationSeekCtx->status != B_PlaybackMpegDashSeekStatus_eValid) {

        /* Advance the (previously) current representation's seek
         * context by the duration of the current segment.*/
        newTimeInTimescaleUnits = representationSeekCtx->periodTimeInTimescaleUnits + representationSeekCtx->durationInTimescaleUnits;
        PRINTMSG_SEEK(("%s:%d: newTimeInTimescaleUnits="BDBG_UINT64_FMT, myFuncName, __LINE__, BDBG_UINT64_ARG(newTimeInTimescaleUnits)));
        PRINTMSG_SEEK(("%s:%d: representationSeekCtx->periodTimeInTimescaleUnits=" BDBG_UINT64_FMT, myFuncName, __LINE__, BDBG_UINT64_ARG(representationSeekCtx->periodTimeInTimescaleUnits)));

        errCode = B_PlaybackIp_MpegDashSeekInRepresentationByTimescale(playback_ip, representationInfo , representationSeekCtx, newTimeInTimescaleUnits);

        if (errCode == B_ERROR_SUCCESS) {
            representationSeekCtx->status = B_PlaybackMpegDashSeekStatus_eValid;    /* Seek context is now valid. */
        }
        else if (errCode == B_ERROR_INVALID_PARAMETER) {
            PRINTMSG_SEEK(("%s:%d : Tried to advance past EOF, retrying later. ", myFuncName, __LINE__   ));
            representationSeekCtx->status = B_PlaybackMpegDashSeekStatus_eInvalid;  /* Invalidate seek context */
        }
        else if (errCode) {
            BDBG_ERR(("%s: Can't seek to next segment in Representation", myFuncName));
            errCode = BERR_TRACE(errCode);
        }
    }


    PRINTMSG_SEEK(("%s:%d: Exit: Seek is at Repr:%p id:%s timeInTs:%"PRIu64" status:%s",
                   myFuncName, __LINE__,
                   (void *)representationSeekCtx->representationInfo,
                   representationSeekCtx->representationId,
                   representationSeekCtx->periodTimeInTimescaleUnits,
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eInvalid    ? "Invalid"   :
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eValid      ? "Valid"     :
                   representationSeekCtx->status == B_PlaybackMpegDashSeekStatus_eDownloaded ? "Downloaded":
                                                                                               "<Unknown>"  ));
    /* GARYWASHERE: Need to update seek time in requestSeekCtx... */
    /* requestSeekCtx->periodTime = ????? */
    return errCode;
}


/*****************************************************************************
 * Update a SeekContext to correspond to a given MPD time. This will
 * also update the individual RepresentationSeekContexts that are contained
 * withing the SeekContext.
 *
 * The seekTime is interpreted differently, depending on the type of
 * MPD (static or dynamic).
 *
 * For dynamic MPDs, new content is continually being added and old
 * content is expiring.  So the seekTime is specified as the time (in
 * milliseconds) from the oldest available content.
 *
 * For static MPDs, the seekTime is relative to the start time of the
 * first Period in the MPD.
 *****************************************************************************/
static B_PlaybackIpError
B_PlaybackIp_MpegDashSeekToMpdTime(B_PlaybackIpHandle playback_ip, MpegDashSeekContext *requestSeekCtx, unsigned seekTimeInMs)
{
    MpegDashSessionState        * mpegDashSession           = playback_ip->mpegDashSessionState;
    MpegDashMpdInfo             * mpdInfo                   = mpegDashSession->mpdInfo;
    MpegDashPeriodInfo          * periodInfo                = NULL;
    MpegDashAdaptationSetInfo   * audioAdaptationSet        = NULL;
    MpegDashAdaptationSetInfo   * videoAdaptationSet        = NULL;
    MpegDashRepresentationInfo  * audioRepresentationInfo   = NULL;
    MpegDashRepresentationInfo  * videoRepresentationInfo   = NULL;
    B_PlaybackIpError             errCode                   = B_ERROR_UNKNOWN;
    uint64_t                      mpdSeekTimeInMs;
    uint64_t                      periodSeekTimeInMs;
    unsigned                      availableBandwidthInKbps;

    /* Get the current network bandwidth... but don't allow it to exceed the
     * maximum available bandwidth.  */
    availableBandwidthInKbps = B_PlaybackIp_MpegDashGetCurrentBandwidth(&mpegDashSession->bandwidthContext);

    mpdSeekTimeInMs = seekTimeInMs;

    PRINTMSG_SEEK(("%s:%d: mpdSeekTimeInMs=%"PRIu64, __FUNCTION__, __LINE__, mpdSeekTimeInMs ));

    /* For "dynamic" MPDs, we need to convert the seek time from being
     * relative to the start of the timeshift buffer to being relative to the
     * start of the MPD, which should be MPD@availabilityStartTime.  */
    if (mpdInfo->isDynamic) {
        MpegDashDateTime  nowDateTime;
        MpegDashDuration  durationSinceMpdAvailable;
        uint64_t   earliestContentTimeInMs;
        uint64_t   mostRecentContentTimeInMs;
        uint64_t   timeShiftBufferDepthInMs;
        uint64_t   suggestedPresentationDelayInMs = 10 * 1000; /* Default suggestedPresentationDelay. */

        /*  Get the time of the most recent content (which is NOW). */
         B_PlaybackIp_MpegDashGetDateTimeNow(&nowDateTime);
         B_PlaybackIp_MpegDashGetDateTimeDifference(&nowDateTime, &mpdInfo->cooked.availabilityStartTime, &durationSinceMpdAvailable);
         B_PlaybackIp_MpegDashGetMsFromDuration(&durationSinceMpdAvailable, &mostRecentContentTimeInMs );

         PRINTMSG_SEEK(("nowDateTime:               " MPEG_DASH_DATETIME_TO_PRINTF_ARGS((&nowDateTime))  ));
         PRINTMSG_SEEK(("availabilityStartTime:     " MPEG_DASH_DATETIME_TO_PRINTF_ARGS((&mpdInfo->cooked.availabilityStartTime))  ));
         PRINTMSG_SEEK(("durationSinceMpdAvailable: " MPEG_DASH_DURATION_TO_PRINTF_ARGS((&durationSinceMpdAvailable))  ));
         PRINTMSG_SEEK(("SeekToMpdTime:  " MPEG_DASH_DURATION_TO_PRINTF_ARGS((&durationSinceMpdAvailable))  ));
         PRINTMSG_SEEK(("%s:%d: mostRecentContentTimeInMs=%"PRIu64, __FUNCTION__, __LINE__, mostRecentContentTimeInMs ));

         /* Subtract the duration of the timeshift buffer. */
         if (mpdInfo->cooked.timeShiftBufferDepth.valid) {
             B_PlaybackIp_MpegDashGetMsFromDuration(&mpdInfo->cooked.timeShiftBufferDepth, &timeShiftBufferDepthInMs );
         }
         else {
             timeShiftBufferDepthInMs = mostRecentContentTimeInMs;  /* If not specified, assume infinite. */
         }
         PRINTMSG_SEEK(("%s:%d: timeShiftBufferDepthInMs=%"PRIu64, __FUNCTION__, __LINE__, timeShiftBufferDepthInMs ));

         if (mostRecentContentTimeInMs <= timeShiftBufferDepthInMs) {
             earliestContentTimeInMs = 0;   /* Not enough to fill timeshift buffer yet. */
         }
         else {
             earliestContentTimeInMs = mostRecentContentTimeInMs - timeShiftBufferDepthInMs;
         }
         PRINTMSG_SEEK(("%s:%d: earliestContentTimeInMs=%"PRIu64, __FUNCTION__, __LINE__, earliestContentTimeInMs ));

         /* And finally, since this function is called for seeking from
          * the external world (as opposed to internally generated
          * seeks for advancing segments), don't allow seeking to within
          * "suggestedPresentationDelay" of the end of the content. */
         if (mpdInfo->cooked.suggestedPresentationDelay.valid) {
             if (B_PlaybackIp_MpegDashGetMsFromDuration(&mpdInfo->cooked.suggestedPresentationDelay, &suggestedPresentationDelayInMs )) {
                 PRINTMSG_SEEK(("SeekToMpdTime:  Got MPD@suggestedPresentationDelay: %"PRIu64" ms", suggestedPresentationDelayInMs));
             }
         }
         else
         {
             PRINTMSG_SEEK(("SeekToMpdTime:  Using default MPD@suggestedPresentationDelay: %"PRIu64" ms", suggestedPresentationDelayInMs));
         }
         if (mostRecentContentTimeInMs > suggestedPresentationDelayInMs) {
             mostRecentContentTimeInMs -= suggestedPresentationDelayInMs;
         }
         else {
             mostRecentContentTimeInMs = 0;
         }

         if (mostRecentContentTimeInMs < earliestContentTimeInMs) {
             mostRecentContentTimeInMs = earliestContentTimeInMs;
         }

         PRINTMSG_SEEK(("%s:%d: suggestedPresentationDelayInMs=%"PRIu64" adjusted mostRecentContentTimeInMs=%"PRIu64"", __FUNCTION__, __LINE__, suggestedPresentationDelayInMs, mostRecentContentTimeInMs ));

         mpdSeekTimeInMs += earliestContentTimeInMs;
         if (mpdSeekTimeInMs > mostRecentContentTimeInMs) {
             mpdSeekTimeInMs = mostRecentContentTimeInMs;  /* Limit seek to end of timeshift buffer. */
         }
    }

    PRINTMSG_SEEK(("%s:%d: Seeking to mpdSeekTimeInMs=%"PRIu64, __FUNCTION__, __LINE__, mpdSeekTimeInMs ));

    periodInfo = B_PlaybackIp_MpegDashSelectPeriod(playback_ip, mpdInfo, mpdSeekTimeInMs);
    if (!periodInfo)  {   /* Didn't find a usable Period */
        BDBG_ERR(("%s: Can't find a usable Period", __FUNCTION__));
        errCode = B_ERROR_INVALID_PARAMETER;    /* Seek time is out of range. */
        goto seekError;
    }

    /* seekTime is relative to the start of the MPD.  Now subtract the
     * starting time of the Period so we'll have time relative to the start of the
     * Period.  */
    periodSeekTimeInMs = mpdSeekTimeInMs - periodInfo->periodStartInMs;

    PRINTMSG_SEEK(("%s:%d: periodInfo->periodStartInMs=%"PRIu64"", __FUNCTION__, __LINE__, periodInfo->periodStartInMs));
    PRINTMSG_SEEK(("%s:%d: Seeking to periodSeekTimeInMs=%"PRIu64, __FUNCTION__, __LINE__, periodSeekTimeInMs ));

    /* Then select audio and video AdaptationSets from the Period. */
    if (!B_PlaybackIp_MpegDashSelectAdaptationSets(
                playback_ip, periodInfo, &videoAdaptationSet, &audioAdaptationSet)) goto seekError;

    if (!videoAdaptationSet && !audioAdaptationSet) {   /* Didn't find any usable AdaptationSets */
        BDBG_ERR(("%s: Can't find any usable AdaptationSets", __FUNCTION__));
        errCode = BERR_TRACE(B_ERROR_UNKNOWN);
        goto seekError;
    }

    /* Now select Representation(s) from the AdaptationSet(s) based on how
     * much available bandwidth we have. */
    if (!B_PlaybackIp_MpegDashSelectRepresentations(playback_ip,
                                                    audioAdaptationSet, videoAdaptationSet,
                                                    &audioRepresentationInfo, &videoRepresentationInfo,
                                                    availableBandwidthInKbps) ) {
        BDBG_ERR(("%s: Can't find any usable Audio Representations", __FUNCTION__));
        errCode = BERR_TRACE(B_ERROR_UNKNOWN);
        goto seekError;
    }

    B_PlaybackIp_MpegDashSetPlaypumpHandles(playback_ip, audioAdaptationSet, videoAdaptationSet);

    /* Now find the segment(s) within the Representation(s) that are within the specified time. */
    /* Find video segment. */
    BKNI_Memset(requestSeekCtx, 0, sizeof (*requestSeekCtx));
    requestSeekCtx->mpdSeekTimeInMs    = seekTimeInMs;
    requestSeekCtx->periodSeekTimeInMs = periodSeekTimeInMs;

    requestSeekCtx->video.status = B_PlaybackMpegDashSeekStatus_eNotInUse;
    requestSeekCtx->audio.status = B_PlaybackMpegDashSeekStatus_eNotInUse;

    requestSeekCtx->currentRepresentationSeekContext = NULL;
    if (videoRepresentationInfo) {
        MpegDashRepresentationSeekContext  * representationSeekCtx = &requestSeekCtx->video;

        PRINTMSG_SEEK(("%s:%d: Seeking in Video Representation %s for periodSeekTimeInMs=%"PRIu64"", __FUNCTION__, __LINE__, videoRepresentationInfo->id, periodSeekTimeInMs));
        errCode = B_PlaybackIp_MpegDashSeekInRepresentationByMs(playback_ip, videoRepresentationInfo, representationSeekCtx, periodSeekTimeInMs);
        if (errCode) {
            BDBG_ERR(("%s: Can't find seek time in Representation", __FUNCTION__));
            errCode = BERR_TRACE(errCode);
            goto seekError;
        }
        representationSeekCtx->status = B_PlaybackMpegDashSeekStatus_eValid;
        requestSeekCtx->currentRepresentationSeekContext = representationSeekCtx;
    }

    if (audioRepresentationInfo && audioRepresentationInfo != videoRepresentationInfo) {
        MpegDashRepresentationSeekContext  * representationSeekCtx = &requestSeekCtx->audio;

        PRINTMSG_SEEK(("%s:%d: Seeking in Audio Representation %s for periodSeekTimeInMs=%"PRIu64"", __FUNCTION__, __LINE__, audioRepresentationInfo->id, periodSeekTimeInMs));
        errCode = B_PlaybackIp_MpegDashSeekInRepresentationByMs(playback_ip, audioRepresentationInfo, representationSeekCtx, periodSeekTimeInMs);
        if (errCode) {
            BDBG_ERR(("%s: Can't find seek time in Representation", __FUNCTION__));
            errCode = BERR_TRACE(errCode);
            goto seekError;
        }
        representationSeekCtx->status = B_PlaybackMpegDashSeekStatus_eValid;
        if (!requestSeekCtx->currentRepresentationSeekContext ) requestSeekCtx->currentRepresentationSeekContext = representationSeekCtx;
    }

    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->video.representationId=%s", __FUNCTION__, __LINE__, requestSeekCtx->video.representationId));
    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->video.periodTime=%"PRIu64, __FUNCTION__, __LINE__, requestSeekCtx->video.periodTimeInMs));
    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->video.byteOffset=%u", __FUNCTION__, __LINE__, requestSeekCtx->video.byteOffset));
    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->video.segmentNumber=%u", __FUNCTION__, __LINE__, requestSeekCtx->video.segmentNumber));
    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->video.segmmentUrlInfo=%p", __FUNCTION__, __LINE__, (void *)requestSeekCtx->video.segmentUrlInfo));

    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->audio.representationId=%s", __FUNCTION__, __LINE__, requestSeekCtx->audio.representationId));
    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->audio.periodTime=%"PRIu64, __FUNCTION__, __LINE__, requestSeekCtx->audio.periodTimeInMs));
    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->audio.byteOffset=%u", __FUNCTION__, __LINE__, requestSeekCtx->audio.byteOffset));
    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->audio.segmentNumber=%u", __FUNCTION__, __LINE__, requestSeekCtx->audio.segmentNumber));
    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->audio.segmmentUrlInfo=%p", __FUNCTION__, __LINE__, (void *)requestSeekCtx->audio.segmentUrlInfo));
    PRINTMSG_SEEK(("%s:%d: requestSeekCtx->currentRepresentation=%p", __FUNCTION__, __LINE__, (void *)requestSeekCtx->currentRepresentationSeekContext));

    return B_ERROR_SUCCESS;

seekError:
    return errCode;   /* failure */
}


/*****************************************************************************
 * Destroy and segment buffers.
 *****************************************************************************/
static void
B_PlaybackIp_MpegDashSegmentBufferDestroy(
    B_PlaybackIpHandle playback_ip
    )
{
    int i;
    MpegDashSessionState *mpegDashSession = playback_ip->mpegDashSessionState;

    for (i=0; i<MPEG_DASH_NUM_SEGMENT_BUFFERS && mpegDashSession->segmentBuffer[i].buffer; i++) {
        if (mpegDashSession->segmentBuffer[i].lock) {
            BKNI_DestroyMutex(mpegDashSession->segmentBuffer[i].lock);
            mpegDashSession->segmentBuffer[i].lock = NULL;
        }
    }
}


/*****************************************************************************
 * Destroy, deallocate, cleanup the MPEG-DASH session.
 *****************************************************************************/
void
B_PlaybackIp_MpegDashSessionDestroy(B_PlaybackIpHandle playback_ip)
{
    MpegDashSessionState *mpegDashSession = playback_ip->mpegDashSessionState;

    BDBG_ENTER(B_PlaybackIp_MpegDashSessionDestroy);

    if (!mpegDashSession)
    {
        BDBG_LEAVE(B_PlaybackIp_MpegDashSessionDestroy);
        return;
    }

    if (mpegDashSession->absoluteUri) {
        BKNI_Free(mpegDashSession->absoluteUri);
        mpegDashSession->absoluteUri = NULL;
    }
    if (mpegDashSession->server) {
        BKNI_Free(mpegDashSession->server);
        mpegDashSession->server = NULL;
    }

    if (mpegDashSession->playbackThreadWakeEvent) {
        BKNI_DestroyEvent(mpegDashSession->playbackThreadWakeEvent);
        mpegDashSession->playbackThreadWakeEvent = NULL;
    }
    if (mpegDashSession->downloadThreadWakeEvent) {
        BKNI_DestroyEvent(mpegDashSession->downloadThreadWakeEvent);
        mpegDashSession->downloadThreadWakeEvent = NULL;
    }
    if (mpegDashSession->reDownloadMpdEvent) {
        BKNI_DestroyEvent(mpegDashSession->reDownloadMpdEvent);
        mpegDashSession->reDownloadMpdEvent = NULL;
    }
    if (mpegDashSession->segDownloadThreadPausedEvent) {
        BKNI_DestroyEvent(mpegDashSession->segDownloadThreadPausedEvent);
        mpegDashSession->segDownloadThreadPausedEvent = NULL;
    }
    if (mpegDashSession->playbackThreadPausedEvent) {
        BKNI_DestroyEvent(mpegDashSession->playbackThreadPausedEvent);
        mpegDashSession->playbackThreadPausedEvent = NULL;
    }
    if (mpegDashSession->segDownloadThreadPauseDoneEvent) {
        BKNI_DestroyEvent(mpegDashSession->segDownloadThreadPauseDoneEvent);
        mpegDashSession->segDownloadThreadPauseDoneEvent = NULL;
    }
    if (mpegDashSession->playbackThreadPauseDoneEvent) {
        BKNI_DestroyEvent(mpegDashSession->playbackThreadPauseDoneEvent);
        mpegDashSession->playbackThreadPauseDoneEvent = NULL;
    }

    if (mpegDashSession->factory)
    {
        batom_factory_destroy(mpegDashSession->factory);
        mpegDashSession->factory = NULL;
    }

    if (mpegDashSession->lock) {
        BKNI_DestroyMutex(mpegDashSession->lock);
        mpegDashSession->lock = NULL;
    }
    B_PlaybackIp_MpegDashSegmentBufferDestroy(playback_ip);

    if (mpegDashSession->mpdSocketFd) {
        close(mpegDashSession->mpdSocketFd);
        mpegDashSession->mpdSocketFd = 0;
    }

    if (mpegDashSession->mpdParseBuffer) {
        BKNI_Free(mpegDashSession->mpdParseBuffer);
        mpegDashSession->mpdParseBuffer = NULL;
    }

    if (mpegDashSession->mpdInfo) {
        /* Destroy the MPD object tree. */
        B_PlaybackIp_MpegDashDestroyMpd(mpegDashSession->mpdInfo);
        mpegDashSession->mpdInfo = NULL;
    }

    if (mpegDashSession->xmlElemRoot) {
        /* Destroy the tree of XML objects to free its memory. */
        B_PlaybackIp_Xml_Destroy( mpegDashSession->xmlElemRoot );
        mpegDashSession->xmlElemRoot = NULL;
    }

    BKNI_Free(mpegDashSession);
    playback_ip->mpegDashSessionState = NULL;

    BDBG_LEAVE(B_PlaybackIp_MpegDashSessionDestroy);
}


/*****************************************************************************
 * Do any cleanup for the MPEG-DASH session that might require
 * access to the Nexus handles or the network socket.  Final
 * cleanup can be done in B_PlaybackIp_MpegDashSessionDestroy().
 *****************************************************************************/
void
B_PlaybackIp_MpegDashSessionStop(B_PlaybackIpHandle playback_ip)
{
    MpegDashSessionState          * mpegDashSession = playback_ip->mpegDashSessionState;

    MpegDashSlavePidEntry         * slavePidEntry;
    struct mpegDashSlavePidHead   * slavePidHead;
    NEXUS_PidChannelHandle          masterPidChannel;
    NEXUS_PidType                   pidType;

    /* Close all existing audio slave PidChannels. */
    pidType = NEXUS_PidType_eAudio;
    slavePidHead = &mpegDashSession->audioSlavePidHead;
    masterPidChannel   = playback_ip->nexusHandles.audioPidChannel;

    /* coverity[use_after_free] */
    for(slavePidEntry = BLST_Q_FIRST(slavePidHead) ; slavePidEntry ; slavePidEntry = BLST_Q_FIRST(slavePidHead) )
    {
        B_PlaybackIp_MpegDashRemoveSlavePidChannel(playback_ip, pidType, slavePidEntry);
    }

    /* Close all existing video slave PidChannels. */
    pidType = NEXUS_PidType_eVideo;
    slavePidHead = &mpegDashSession->videoSlavePidHead;
    masterPidChannel   = playback_ip->nexusHandles.videoPidChannel;

    /* coverity[use_after_free] */
    for(slavePidEntry = BLST_Q_FIRST(slavePidHead) ; slavePidEntry ; slavePidEntry = BLST_Q_FIRST(slavePidHead) )
    {
        B_PlaybackIp_MpegDashRemoveSlavePidChannel(playback_ip, pidType, slavePidEntry);
    }
}


/*****************************************************************************
 * Set up the MPEG-DASH session.  Allocate and initialize
 * data structures, finish download of the MPD file (which was started by
 * B_PlaybackIp_HttpSessionOpen).  Then download some of the content and
 * determines the stream information (PSI).
 *****************************************************************************/
int
B_PlaybackIp_MpegDashSessionSetup(B_PlaybackIpHandle playback_ip, char *http_hdr)
{
    char *pValue;
    MpegDashMpdInfo *mpdInfo;
    MpegDashSessionState   * mpegDashSession;
    MpegDashSegmentBuffer  * segmentBuffer = NULL;
    int initial_data_len = 0;

    BDBG_ENTER(B_PlaybackIp_MpegDashSessionSetup);

    if (B_PlaybackIp_IsMpegDashSession(playback_ip->openSettings.socketOpenSettings.url, http_hdr) != true) {
        PRINTMSG_SESSION(("%s: Not an MPEG-DASH Streaming Session", __FUNCTION__));
        BDBG_LEAVE(B_PlaybackIp_MpegDashSessionSetup);
        return 0;
    }

    /* Now we know (or think) it is an MPEG-DASH session */
    PRINTMSG_SESSION(("%s: MPEG-DASH Session: download & parse MPD file", __FUNCTION__));

    /* allocate MPEG-DASH session state */
    if ((playback_ip->mpegDashSessionState = (MpegDashSessionState *)BKNI_Malloc(sizeof(MpegDashSessionState))) == NULL) {
        BDBG_ERR(("%s: failed to allocate %d bytes for MPEG-DASH Session state", __FUNCTION__, (int)sizeof(MpegDashSessionState)));
        goto error;
    }
    mpegDashSession = playback_ip->mpegDashSessionState;
    BKNI_Memset(mpegDashSession, 0, sizeof(MpegDashSessionState));

    /* allocate a buffer where MPD file will be completely downloaded */
    if ((mpegDashSession->mpdParseBuffer = (char *)BKNI_Malloc(MPEG_DASH_MPD_FILE_SIZE)) == NULL) {
        BDBG_ERR(("%s: failed to allocate %d bytes for downloading the MPD file", __FUNCTION__, MPEG_DASH_MPD_FILE_SIZE));
        goto error;
    }
    mpegDashSession->mpdParseBufferSize = MPEG_DASH_MPD_FILE_SIZE;

    B_PlaybackIp_MpegDashResetBandwidthSample(&mpegDashSession->bandwidthContext, 10); /* Use 10-sample rolling average */

    pValue = getenv("max_download_bitrate");
    if (pValue) {
        mpegDashSession->maxNetworkBandwidth = strtoul(pValue, NULL, 0);
    }
#define PLAYBACK_IP_MAX_NETWORK_BANDWIDTH 5800000 /* set to 1.8Mpbs */
    if (!pValue || mpegDashSession->maxNetworkBandwidth == 0) {
        mpegDashSession->maxNetworkBandwidth = PLAYBACK_IP_MAX_NETWORK_BANDWIDTH;
    }
    PRINTMSG_SESSION(("%s: max network bandwidth set to %d", __FUNCTION__, mpegDashSession->maxNetworkBandwidth));


    /* copy any initial payload data (read part of the initial HTTP response) into the MPD buffer */
    initial_data_len = playback_ip->chunkPayloadLength ? playback_ip->chunkPayloadLength : playback_ip->initial_data_len;
    if (initial_data_len) {
        memcpy(mpegDashSession->mpdParseBuffer, playback_ip->temp_buf, initial_data_len);
        mpegDashSession->mpdParseBufferDepth = initial_data_len;
        playback_ip->initial_data_len = 0;
        playback_ip->chunkPayloadLength = 0;
    }
    else {
        mpegDashSession->mpdParseBufferDepth = 0;
    }

    PRINTMSG_SESSION(("%s:%d: mpegDashSession->mpdParseBufferDepth=%d", __FUNCTION__, __LINE__, mpegDashSession->mpdParseBufferDepth));
    PRINTMSG_SESSION(("%s:%d: mpegDashSession->mpdParseBufferSize=%d", __FUNCTION__, __LINE__, mpegDashSession->mpdParseBufferSize));
    PRINTMSG_SESSION(("%s:%d: playback_ip->socketState.fd=%d", __FUNCTION__, __LINE__, playback_ip->socketState.fd));

    /* Download the remainder of the MPD file into the mpdParseBuffer */
    if (B_PlaybackIp_MpegDashFinishFileDownload(playback_ip, playback_ip->socketState.fd, mpegDashSession->mpdParseBuffer, mpegDashSession->mpdParseBufferSize, &mpegDashSession->mpdParseBufferDepth, true) != true) {
        BDBG_ERR(("%s: failed to download the MPD file", __FUNCTION__));
        goto error;
    }
    PRINTMSG_SESSION(("%s: Finished downloading MPD file of %d bytes", __FUNCTION__, mpegDashSession->mpdParseBufferDepth));

    /* Create the session lock mutex. */
    if (BKNI_CreateMutex(&mpegDashSession->lock) != 0) {
        BDBG_ERR(("%s: Failed to create BKNI mutex at %d", __FUNCTION__, __LINE__));
        goto error;
    }

    /* Create that batom factory that we'll user for
     * probing/parsing. */
    mpegDashSession->factory = batom_factory_create(bkni_alloc, 16);
    if( ! mpegDashSession->factory) {
        BDBG_ERR(("%s: Failed to create a batom_factory", __FUNCTION__));
        goto error;
    }

    /* Hijack the index and data buffers so we can use them as segment download buffers. */
    if (B_PlaybackIp_MpegDashSegmentBufferSetup(playback_ip) < 0) {
        goto error;
    }

    /* Initialize the slave pid lists. */
    BLST_Q_INIT(&mpegDashSession->audioSlavePidHead);
    BLST_Q_INIT(&mpegDashSession->videoSlavePidHead);

    PRINTMSG_SESSION(("%s:%d: Before: mpegDashSession->absoluteUri=%s", __FUNCTION__, __LINE__, mpegDashSession->absoluteUri));
    PRINTMSG_SESSION(("%s:%d: playback_ip->openSettings.socketOpenSettings.url=%s", __FUNCTION__, __LINE__, playback_ip->openSettings.socketOpenSettings.url));
    PRINTMSG_SESSION(("%s:%d: playback_ip->openSettings.socketOpenSettings.port=%u", __FUNCTION__, __LINE__, playback_ip->openSettings.socketOpenSettings.port));
    PRINTMSG_SESSION(("%s:%d: playback_ip->openSettings.socketOpenSettings.ipAddr=%s", __FUNCTION__, __LINE__, playback_ip->openSettings.socketOpenSettings.ipAddr));


    /* build the uri of the MPD file from the playback_ip open settings */
    if ((mpegDashSession->absoluteUri = B_PlaybackIp_MpegDashBuildAbsoluteUri(playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, ""/* empty base uri*/, playback_ip->openSettings.socketOpenSettings.url)) == NULL) {
        BDBG_ERR(("Failed to build URI at %s:%d", __FUNCTION__, __LINE__));
        goto error;
    }

    PRINTMSG_SESSION(("%s:%d: After: mpegDashSession->absoluteUri=%s", __FUNCTION__, __LINE__, mpegDashSession->absoluteUri));
    PRINTMSG_SESSION(("%s:%d: playback_ip->openSettings.socketOpenSettings.url=%s", __FUNCTION__, __LINE__, playback_ip->openSettings.socketOpenSettings.url));
    PRINTMSG_SESSION(("%s:%d: playback_ip->openSettings.socketOpenSettings.port=%u", __FUNCTION__, __LINE__, playback_ip->openSettings.socketOpenSettings.port));
    PRINTMSG_SESSION(("%s:%d: playback_ip->openSettings.socketOpenSettings.ipAddr=%s", __FUNCTION__, __LINE__, playback_ip->openSettings.socketOpenSettings.ipAddr));


    /* Now parse that uri into components and put them into the mpegDashSession structure */
    if ((mpegDash_parse_absolute_url(&mpegDashSession->protocol, &mpegDashSession->server, &mpegDashSession->port, &mpegDashSession->uri, mpegDashSession->absoluteUri) == false) || mpegDashSession->protocol != B_PlaybackIpProtocol_eHttp) {
        BDBG_ERR(("Failed to parse URI at %s:%d", __FUNCTION__, __LINE__));
        goto error;
    }

    /* initialize the queue */
    /* GARYWASHERE: I think the queue must be empty or else we need to free any Representations that are present. */
    BLST_Q_INIT(&mpegDashSession->mpegDashRepresentationInfoHead);  /* GARYWASHERE: This should probably go away. */

    /* Parse the MPD file into an XML object tree, then from there into our C-based object tree. */
    if (!B_PlaybackIp_MpegDashParseMpd(playback_ip, &mpdInfo)) {
        BDBG_ERR(("%s: Failed to parse MPD file", __FUNCTION__));
        goto error;
    }
    mpegDashSession->mpdInfo = mpdInfo;

    /* Position ourselves to the correct Period and Segment for the desired start time. */
    if (B_PlaybackIp_MpegDashSeekToMpdTime(playback_ip, &mpegDashSession->requestSeekCtx, 0)) {
        BDBG_ERR(("%s: Failed to seek to initial position", __FUNCTION__));
        goto error;
    }

    if (mpegDashSession->requestSeekCtx.video.representationInfo) {
        /* At session setup nobody should be using the segment buffers, so the
         * allocate should never fail here. */
        segmentBuffer = B_PlaybackIp_MpegDashSegmentBufferAlloc(playback_ip);
        if (!segmentBuffer) {
            BDBG_ERR(("%s: Failed to allocate segmentBuffer for probing stream info!", __FUNCTION__));
            goto error;
        }

        /* We need to probe the video content so we can get the PSI info.  */
        if (!B_PlaybackIp_MpegDashProbeForRepresentationStreamInfo(playback_ip, &mpegDashSession->requestSeekCtx.video, segmentBuffer)) {
            BDBG_ERR(("%s: Failed to probe for Representation's stream info", __FUNCTION__));
            goto error;
        }

        /* Since we're going to return the videoPid back to the application,
         * then it's going to open a pid channel for it, we'll just save
         * the pid here so we can use it later.  */
        mpegDashSession->videoMasterPid = playback_ip->psi.videoPid;

        B_PlaybackIp_MpegDashSegmentBufferFree(playback_ip, segmentBuffer);
        segmentBuffer = NULL;
    }

    if (mpegDashSession->requestSeekCtx.audio.representationInfo) {
        segmentBuffer = B_PlaybackIp_MpegDashSegmentBufferAlloc(playback_ip);
        if (!segmentBuffer) {
            BDBG_ERR(("%s: Failed to allocate segmentBuffer for probing stream info!", __FUNCTION__));
            goto error;
        }

        /* We need to probe the audio content so we can get the PSI info.  */
        if (!B_PlaybackIp_MpegDashProbeForRepresentationStreamInfo(playback_ip, &mpegDashSession->requestSeekCtx.audio, segmentBuffer)) {
            BDBG_ERR(("%s: Failed to probe for Representation's stream info", __FUNCTION__));
            goto error;
        }

        /* Since we're going to return the audioPid back to the application,
         * then it's going to open a pid channel for it, we'll just save
         * the pid here so we can use it later.  */
        mpegDashSession->audioMasterPid = playback_ip->psi.audioPid;

        B_PlaybackIp_MpegDashSegmentBufferFree(playback_ip, segmentBuffer);
        segmentBuffer = NULL;
    }

    {
        /* Now we can decide whether audio and video can share the
         * same Nexus playpump, or if they need separate ones.  If
         * audio and video are in different Adaptation Sets, then it's
         * possible that they might have the same Track ID, and
         * therefore will need to use different playpumps. */

        bool                          usePlaypump2ForAudio    = false;
        B_PlaybackIpPsiInfo         * psi                     = &playback_ip->psi;
        MpegDashRepresentationInfo  * audioRepresentationInfo = mpegDashSession->requestSeekCtx.audio.representationInfo;
        MpegDashRepresentationInfo  * videoRepresentationInfo = mpegDashSession->requestSeekCtx.video.representationInfo;

        if (audioRepresentationInfo && videoRepresentationInfo) {
            if (audioRepresentationInfo->parentObj != videoRepresentationInfo->parentObj) {
                usePlaypump2ForAudio = true;
            }
        }
        psi->usePlaypump2ForAudio = usePlaypump2ForAudio;
        PRINTMSG_SESSION(("%s:%d: usePlaypump2ForAudio: %s", __FUNCTION__, __LINE__, usePlaypump2ForAudio ? "True" : "False" ));
    }


#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: finished parsing the MPD file, now setup for media probing", __FUNCTION__));
#endif

    /* reset the content length, otherwise media probe takes way too long */
    /* TODO: need to see if this is needed or psiParsingTimeLimit will do the trick */
    playback_ip->contentLength = 0;

    if (BKNI_CreateEvent(&mpegDashSession->downloadThreadWakeEvent)) {
        BDBG_ERR(("%s: Failed to create an event", __FUNCTION__));
        goto error;
    }

    if (BKNI_CreateEvent(&mpegDashSession->playbackThreadWakeEvent)) {
        BDBG_ERR(("%s: Failed to create an event", __FUNCTION__));
        goto error;
    }

    if (BKNI_CreateEvent(&mpegDashSession->reDownloadMpdEvent)) {
        BDBG_ERR(("%s: Failed to create an event", __FUNCTION__));
        goto error;
    }

    if (BKNI_CreateEvent(&mpegDashSession->segDownloadThreadPausedEvent)) {
        BDBG_ERR(("%s: Failed to create an event", __FUNCTION__));
        goto error;
    }

    if (BKNI_CreateEvent(&mpegDashSession->playbackThreadPausedEvent)) {
        BDBG_ERR(("%s: Failed to create an event", __FUNCTION__));
        goto error;
    }

    if (BKNI_CreateEvent(&mpegDashSession->segDownloadThreadPauseDoneEvent)) {
        BDBG_ERR(("%s: Failed to create an event", __FUNCTION__));
        goto error;
    }

    if (BKNI_CreateEvent(&mpegDashSession->playbackThreadPauseDoneEvent)) {
        BDBG_ERR(("%s: Failed to create an event", __FUNCTION__));
        goto error;
    }

    playback_ip->mpegDashSessionEnabled = true;
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: Complete, now media probing will start.", __FUNCTION__));
#endif

    /* the actual media probe operation happens in the caller function of http module */
    BDBG_LEAVE(B_PlaybackIp_MpegDashSessionSetup);
    return 0;

error:
    if (segmentBuffer) {
        B_PlaybackIp_MpegDashSegmentBufferFree(playback_ip, segmentBuffer);
    }

    B_PlaybackIp_MpegDashSessionDestroy(playback_ip);
    BDBG_LEAVE(B_PlaybackIp_MpegDashSessionSetup);
    return -1;
}


/*****************************************************************************
 * This function is called after app has filled in the nexusHandles.  So we
 * can use those handles to obtain some information that we'll need later.
 *****************************************************************************/
B_PlaybackIpError
B_PlaybackIp_MpegDashSessionStart(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionStartSettings *startSettings,
    B_PlaybackIpSessionStartStatus *startStatus /* [out] */
    )
{

    B_PlaybackIpError        errorCode = B_ERROR_PROTO;

    if (!playback_ip || !startSettings || !startStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, startSettings %p, startStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)startSettings, (void *)startStatus));
        errorCode = B_ERROR_INVALID_PARAMETER;
        goto error;
    }

    errorCode = B_ERROR_SUCCESS;

error:
    return errorCode;
}


/*****************************************************************************
 * The following functions are used for wrapping the downloaded segments
 * with the appropriate MP4 boxes as required before feeding to NEXUS_Playpump.
 *****************************************************************************/
static void
write_uint8(bfile_io_random_write_t fout, uint8_t d)
{
    fout->write(fout, &d, sizeof (d) );
}

static void
write_uint24_be(bfile_io_random_write_t fout, uint32_t data)
{
    uint8_t d[3];
    d[0] = data >> 16;
    d[1] = data >> 8;
    d[2] = data;
    fout->write(fout, &d, sizeof (d) );
}

static void
write_uint32_be(bfile_io_random_write_t fout, uint32_t data)
{
    uint8_t d[4];
    d[0] = data >> 24;
    d[1] = data >> 16;
    d[2] = data >> 8;
    d[3] = data;
    fout->write(fout, &d, sizeof (d) );
}

static void
write_uint32_le(bfile_io_random_write_t fout, uint32_t data)
{
    uint8_t d[4];
    d[0] = data;
    d[1] = data >> 8;
    d[2] = data >> 16;
    d[3] = data >> 24;
    fout->write(fout, &d, sizeof (d) );
}

static void
write_uint64_be(bfile_io_random_write_t fout, uint64_t data)
{
    uint8_t d[8];
    d[0] = data >> 56;
    d[1] = data >> 48;
    d[2] = data >> 40;
    d[3] = data >> 32;
    d[4] = data >> 24;
    d[5] = data >> 16;
    d[6] = data >> 8;
    d[7] = data;
    fout->write(fout, d, sizeof(d));
}

static void
write_data(bfile_io_random_write_t fout, const uint8_t *data, size_t len)
{
    fout->write(fout, data, len);
}

#define MPEG_DASH_MP4_BOX_SIZE  (8)
static int
start_mp4_box(bfile_io_random_write_t fout, const char *type)
{
    int offset = fout->seek(fout, 0, SEEK_CUR);
    write_uint32_be(fout, 0); /* size */
    write_data(fout, (uint8_t*)type, 4); /* type */
    return offset;
}

static void
finish_mp4_box(bfile_io_random_write_t fout, int offset)
{
    int current = fout->seek(fout, 0, SEEK_CUR);

    fout->seek(fout, offset, SEEK_SET);
    write_uint32_be(fout, current-offset); /* size */
    fout->seek(fout, current, SEEK_SET);
    return;
}

#define MPEG_DASH_MP4_FULLBOX_SIZE  (MPEG_DASH_MP4_BOX_SIZE + 4)
static int
start_mp4_fullbox(bfile_io_random_write_t fout, const char *type, unsigned version, uint32_t flags)
{
    int offset = start_mp4_box(fout, type);
    write_uint8(fout, version);
    write_uint24_be(fout, flags);
    return offset;
}


#define MPEG_DASH_MP4_TREX_BOX_SIZE  (MPEG_DASH_MP4_FULLBOX_SIZE + 5 * 4)
static void
write_trex_box(bfile_io_random_write_t fout,  uint32_t track_ID, uint32_t default_sample_description_index, uint32_t default_sample_duration, uint32_t default_sample_size, uint32_t default_sample_flags)
{
    int offset = start_mp4_fullbox(fout, "trex", 0, 0);
    write_uint32_be(fout, track_ID);
    write_uint32_be(fout, default_sample_description_index);
    write_uint32_be(fout, default_sample_duration);
    write_uint32_be(fout, default_sample_size);
    write_uint32_be(fout, default_sample_flags);
    finish_mp4_box(fout, offset);
    return;
}

#define MPEG_DASH_MP4_BDAT_BOX_EMPTY_SIZE  (MPEG_DASH_MP4_BOX_SIZE)
static void
write_bdat_box(bfile_io_random_write_t fout,  const uint8_t *data, size_t data_len)
{
    int offset = start_mp4_box(fout, "bdat");
    write_data(fout, data, data_len);
    finish_mp4_box(fout, offset);
    return;
}

#define MPEG_DASH_MP4_BHED_BOX_SIZE  (MPEG_DASH_MP4_BOX_SIZE + 8 + 8 + 4)
static void
write_bhed_box(bfile_io_random_write_t fout, uint64_t timescale, uint64_t start_time, uint32_t fourcc)
{
    int offset = start_mp4_box(fout, "bhed");
    write_uint64_be(fout, timescale);
    write_uint64_be(fout, start_time);
    write_uint32_le(fout, fourcc);
    finish_mp4_box(fout, offset);
    return;
}


/*****************************************************************************
 * Do MPEG-DASH specific processing for the B_PlaybackIp_Pause() API.
 *****************************************************************************/
B_PlaybackIpError
B_PlaybackIp_PauseMpegDash(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    BSTD_UNUSED(ipTrickModeSettings);
    if (!playback_ip->mpegDashSessionState) {
        BDBG_ERR(("%s: ERROR: MpegDashSessionState is NULL", __FUNCTION__));
        return B_ERROR_UNKNOWN;
    }

    /* in MPEG-DASH pause, we dont disconnect the server connection and instead keep that going */
    /* We instead pause by setting STC rate to 0, this way download and playback threads still continue until all buffers get full */
#ifdef NEXUS_HAS_SIMPLE_DECODER
    if (playback_ip->nexusHandles.simpleStcChannel) {
        if (NEXUS_SimpleStcChannel_SetRate(playback_ip->nexusHandles.simpleStcChannel, 0, 0 ) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to pause by setting stc rate to 0", __FUNCTION__));
            return B_ERROR_UNKNOWN;
        }
    }
    else
#endif
    if (playback_ip->nexusHandles.stcChannel) {
        if (NEXUS_StcChannel_SetRate(playback_ip->nexusHandles.stcChannel, 0, 0 ) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to pause by setting stc rate to 0", __FUNCTION__));
            return B_ERROR_UNKNOWN;
        }
    }
    else {
        BDBG_ERR(("%s: ERROR: neither simpleStcChannel or stcChannel is set", __FUNCTION__));
        return B_ERROR_UNKNOWN;
    }
    return B_ERROR_SUCCESS;
}


/*****************************************************************************
 * Do MPEG-DASH specific processing for the B_PlaybackIp_Play() API.
 *****************************************************************************/
B_PlaybackIpError
B_PlaybackIp_PlayMpegDash(
    B_PlaybackIpHandle playback_ip
    )
{
    /* just resume the STC rate and we should be good */
#ifdef NEXUS_HAS_SIMPLE_DECODER
    if (playback_ip->nexusHandles.simpleStcChannel) {
        if (NEXUS_SimpleStcChannel_SetRate(playback_ip->nexusHandles.simpleStcChannel, 1, 0 ) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to un-pause by setting stc rate to 0", __FUNCTION__));
            return B_ERROR_UNKNOWN;
        }
    }
    else
#endif
    if (playback_ip->nexusHandles.stcChannel) {
        if (NEXUS_StcChannel_SetRate(playback_ip->nexusHandles.stcChannel, 1, 0 ) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to un-pause by setting stc rate to 0", __FUNCTION__));
            return B_ERROR_UNKNOWN;
        }
    }
    else {
        BDBG_ERR(("%s: ERROR: neither simpleStcChannel or stcChannel is set", __FUNCTION__));
        return B_ERROR_UNKNOWN;
    }
    return B_ERROR_SUCCESS;
}


/*****************************************************************************
 * Do MPEG-DASH specific processing for the B_PlaybackIp_Seek() API.
 *****************************************************************************/
B_PlaybackIpError
B_PlaybackIp_SeekMpegDash(
    B_PlaybackIpHandle playback_ip,
    NEXUS_PlaybackPosition seekPosition
    )
{
    B_PlaybackIpState currentState;
    MpegDashSessionState *mpegDashSession = playback_ip->mpegDashSessionState;
    BERR_Code rc;
    int i;

    PRINTMSG_SEEK(("%s: Validating seekPosition: %lu", __FUNCTION__, seekPosition));

    {
        MpegDashSeekContext   requestSeekCtx;

        if (B_PlaybackIp_MpegDashSeekToMpdTime(playback_ip, &requestSeekCtx, seekPosition)) {
            BDBG_ERR(("%s: Incorrect seekPosition %lu", __FUNCTION__, seekPosition));
            return B_ERROR_INVALID_PARAMETER;
        }
    }

    PRINTMSG_SEEK(("%s:%d : Seek position seems valid", __FUNCTION__, __LINE__   ));

    /* save the current state */
    currentState = playback_ip->playback_state;
    /* now synchronize with the media segment download and playpump feeder threads */
    /* by changing the playback_state to WaitingToEnterTrickMode, these threads will pause their work, */
    /* signal this back to this thread and then wait for seek work to finish. Then, this thread should */
    /* signal these worker threads to resume downloading media segments and feeding to playpump. */


    playback_ip->playback_state = B_PlaybackIpState_eWaitingToEnterTrickMode;
    if (mpegDashSession->downloadThreadWakeEvent)
    {
        PRINTMSG_DOWNLOAD((":%d %s(): Setting downloadThreadWakeEvent.", __LINE__, __FUNCTION__));
        BKNI_SetEvent(mpegDashSession->downloadThreadWakeEvent); /* Wake download thread so it sees the trickmode request */
    }

    PRINTMSG_SEEK(("%s:%d : Waiting for Download thread to pause", __FUNCTION__, __LINE__   ));;

    rc = BKNI_WaitForEvent(mpegDashSession->segDownloadThreadPausedEvent, 10*MPEG_DASH_EVENT_TIMEOUT_MSEC);
    if (rc == BERR_TIMEOUT) {
        BDBG_ERR(("%s: EVENT timeout: failed to receive event from MPEG-DASH Segment download thread indicating its paused", __FUNCTION__));
        return B_ERROR_TIMEOUT;
    } else if (rc!=0) {
        BDBG_ERR(("%s: failed to wait for event from MPEG-DASH Segment download thread indicating its paused, rc = %d", __FUNCTION__, rc));
        return B_ERROR_UNKNOWN;
    }

    PRINTMSG_SEEK(("%s:%d : Download thread seems to be paused ", __FUNCTION__, __LINE__   ));

    if (mpegDashSession->playbackThreadWakeEvent)
    {
        PRINTMSG_PLAYBACK(("%s:%d: Setting playbackThreadWakeEvent so it can see trickmode request.", __FUNCTION__, __LINE__));
        BKNI_SetEvent(mpegDashSession->playbackThreadWakeEvent); /* Wake download thread so it sees the trickmode request */
    }
    /* now pause the playpump feeder thread */
    rc = BKNI_WaitForEvent(mpegDashSession->playbackThreadPausedEvent, 10*MPEG_DASH_EVENT_TIMEOUT_MSEC);
    if (rc == BERR_TIMEOUT) {
        BDBG_ERR(("%s: EVENT timeout: failed to receive event from MPEG-DASH Playpump feeder thread indicating its paused", __FUNCTION__));
        return B_ERROR_TIMEOUT;
    } else if (rc!=0) {
        BDBG_ERR(("%s: failed to wait for event from MPEG-DASH Playpump feeder thread indicating its paused, rc = %d", __FUNCTION__, rc));
        return B_ERROR_UNKNOWN;
    }

    PRINTMSG_SEEK(("%s:%d : Download and Playback threads seem to be paused ", __FUNCTION__, __LINE__   ));

    PRINTMSG_SEEK(("%s:%d : Starting seek in MPD", __FUNCTION__, __LINE__   ));
    if (B_PlaybackIp_MpegDashSeekToMpdTime(playback_ip, &mpegDashSession->requestSeekCtx, seekPosition)) {
        BDBG_ERR(("%s: Incorrect seekPosition %lu", __FUNCTION__, seekPosition));
        return B_ERROR_INVALID_PARAMETER;
    }

    mpegDashSession->downloadedAllSegmentsInCurrentRepresentation = false;

    PRINTMSG_SEEK(("%s:%d : Finished seek in MPD, flushing A/V pipeline", __FUNCTION__, __LINE__   ));

    /* flush the current pipeline so we resume from the new seek location */
    if (B_PlaybackIp_UtilsFlushAvPipeline(playback_ip)) {
        BDBG_ERR(("%s: ERROR: Failed to flush the AV pipeline\n", __FUNCTION__));
        return B_ERROR_UNKNOWN;
    }

    PRINTMSG_SEEK(("%s:%d : Freeing all segment buffers", __FUNCTION__, __LINE__   ));
    for (i=0; i<MPEG_DASH_NUM_SEGMENT_BUFFERS; i++) {
        B_PlaybackIp_MpegDashSegmentBufferFree(playback_ip, &mpegDashSession->segmentBuffer[i]);
    }

    playback_ip->playback_state = currentState;

    /* Resume the Download and Playback threads. */
    PRINTMSG_SEEK(("%s:%d : Resuming Download and Playback threads", __FUNCTION__, __LINE__   ));
    BKNI_SetEvent(mpegDashSession->segDownloadThreadPauseDoneEvent);
    BKNI_SetEvent(mpegDashSession->playbackThreadPauseDoneEvent);

    playback_ip->lastSeekPosition = mpegDashSession->requestSeekCtx.periodSeekTimeInMs;
    /* GARYWASHERE: is this right??? Maybe it needs to use PTS time?  */

    PRINTMSG_SEEK(("%s: B_PlaybackIp_SeekMpegDash successful to %lu msec", __FUNCTION__, playback_ip->lastSeekPosition));
    return B_ERROR_SUCCESS;
}


/*****************************************************************************
 * Callback function for NEXUS_Playback's dataCallback.  Called when
 * space becomes available in the Playpump.
 *****************************************************************************/
static void
B_PlaybackIp_MpegDashPlaypumpCallback(void *context, int param)
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)context;
    BSTD_UNUSED(param);

    PRINTMSG_PLAYBACK(("%s:%d: Setting playbackThreadWakeEvent because Playpump now has room for more.", __FUNCTION__, __LINE__));
    BKNI_SetEvent(playback_ip->mpegDashSessionState->playbackThreadWakeEvent);
}


extern bool B_PlaybackIp_UtilsEndOfStream( B_PlaybackIpHandle playback_ip);


/******************************************************************************
 * Take a look in the downloaded segment to see if has an "sidx" (Segment Index)
 * box.  If so, then extract the "earliest_presentation_time" so we can use it
 * as the segment's starting time.
 ******************************************************************************/
bool B_PlaybackIp_MpegDashParseSegment(MpegDashSegmentBuffer  * segmentBuffer, uint64_t  * startTime, unsigned long  * startTimeTimescale)
{
    batom_vec vec;
    batom_cursor cursor;
    bmp4_fullbox fullbox;
    uint32_t  reference_id;
    uint32_t  timescale;
    uint64_t  earliest_presentation_time;
    uint64_t  first_offset;

    batom_vec_init(&vec, segmentBuffer->buffer + segmentBuffer->mp4BoxPrefixSize,  segmentBuffer->bufferDepth);
    batom_cursor_from_vec(&cursor, &vec, 1);


    for(;;) {
        bmp4_box box;
        size_t box_hdr_size = bmp4_parse_box(&cursor, &box);
        if(box_hdr_size==0) {
            return false;
        }
        if(box.type == BMP4_TYPE('s','i','d','x')) {
            break;
        }
        if(box.size>box_hdr_size) {
            size_t  skipcount = box.size - box_hdr_size;
            if (skipcount != batom_cursor_skip(&cursor, skipcount) ) {
                PRINTMSG_DOWNLOAD(("%s:%d : Can't find \"sidx\" box", __FUNCTION__, __LINE__ ));
                return false;
            }
        }
    }

    /* Since the sidx box is a "FullBox", parse it to get the version and flags. */
    if (!bmp4_parse_fullbox(&cursor, &fullbox)) {
        BDBG_ERR(("%s:%d : Failed to parse \"sidx\" fullbox", __FUNCTION__, __LINE__ ));
        return false;
    }

    reference_id = batom_cursor_uint32_be (&cursor);
    timescale   =  batom_cursor_uint32_be (&cursor);

    /* The next two fields can be 32 or 64-bit, depending on the version of the "sidx" box. */
    if (fullbox.version == 0) {
        earliest_presentation_time = batom_cursor_uint32_be (&cursor);
        first_offset = batom_cursor_uint32_be (&cursor);

    }
    else {
        earliest_presentation_time = batom_cursor_uint64_be (&cursor);
        first_offset = batom_cursor_uint64_be (&cursor);
    }

     /* There are a few more fields, but nothing that we're interested in...
      * so we're done for now. */
    *startTime = earliest_presentation_time;
    *startTimeTimescale = timescale;

    return true;
}


/*****************************************************************************
 * Figure out how large the Segment Box Prefix is, then adjust the Segment
 * Buffer to leave space for it.
 *****************************************************************************/
bool
B_PlaybackIp_MpegDashAllocateSegmentBoxPrefix( MpegDashSegmentBuffer  * segmentBuffer, MpegDashSeekContext  * requestSeekCtx )
{
    MpegDashRepresentationInfo  * representationInfo = requestSeekCtx->currentRepresentationSeekContext->representationInfo;
    size_t          codecSpecificSize;

    if (requestSeekCtx->currentRepresentationSeekContext == &requestSeekCtx->audio) {
        codecSpecificSize = representationInfo->codecSpecificSizeForAudio;
    }
    else if (requestSeekCtx->currentRepresentationSeekContext == &requestSeekCtx->video) {
        codecSpecificSize = representationInfo->codecSpecificSizeForVideo;
    }
    else {
        BDBG_ERR(("%s: Unable to determine media type of segment!", __FUNCTION__));
        goto error;
    }


    /* Calculate (and remember) the size of the Segment Prefix boxes. */
    segmentBuffer->mp4BoxPrefixSize = MPEG_DASH_MP4_BOX_SIZE             +   /* "bmp4" box */
                                      MPEG_DASH_MP4_BHED_BOX_SIZE        +   /* "bhed" box */
                                      MPEG_DASH_MP4_BDAT_BOX_EMPTY_SIZE  +   /* empty "bdat" box */
                                      codecSpecificSize                  +   /* data in "bdat" box */
                                      MPEG_DASH_MP4_TREX_BOX_SIZE;           /* "trex" box  */

    /* Now adjust the bufferDepth to allocate space for the boxes.  We'll come back and
     * fill in the space with the boxes after we've downloaded the segment. */
    segmentBuffer->bufferDepth = segmentBuffer->mp4BoxPrefixSize;

    return true; /* normal success */

error:
    return false;
}



/*****************************************************************************
 * Build and insert a prefix into the Segment Buffer so the segment will have
 * the mp4 box structure needed by NEXUS_Playpump for fragmented MP4 streams.
 *****************************************************************************/
bool
B_PlaybackIp_MpegDashBuildSegmentBoxPrefix( MpegDashSegmentBuffer  * segmentBuffer, MpegDashSeekContext  * requestSeekCtx )
{
    bfile_io_random_write_t fout = NULL;
    size_t mp4_box_offset = 0;

    MpegDashRepresentationInfo  * representationInfo = requestSeekCtx->currentRepresentationSeekContext->representationInfo;
    uint32_t        fourcc;
    unsigned char * codecSpecificAddr;
    size_t          codecSpecificSize;
    unsigned long   trackId;

    unsigned long   mdhdTimescale = 0;
    unsigned long   trexDefaultSampleDescriptionIndex;
    unsigned long   trexDefaultSampleDuration;
    unsigned long   trexDefaultSampleFlags;
    unsigned long   trexDefaultSampleSize;
    uint64_t        startTimeInMdhdTimescale;
    uint64_t        startTimeFromSidxBox;
    unsigned long   timescaleFromSidxBox;

    /*------------------------------------------------------------
     *  Collect some infomation from the Representation. We'll
     *  need it to build the boxes.
     *----------------------------------------------------------*/
    if (requestSeekCtx->currentRepresentationSeekContext == &requestSeekCtx->audio) {
        fourcc            = representationInfo->fourccForAudio;
        codecSpecificAddr = representationInfo->codecSpecificAddrForAudio;
        codecSpecificSize = representationInfo->codecSpecificSizeForAudio;
        trackId           = representationInfo->trackIdForAudio;

        mdhdTimescale                       = representationInfo->mdhdTimescaleForAudio;
        trexDefaultSampleDescriptionIndex   = representationInfo->trexDefaultSampleDescriptionIndexForAudio;
        trexDefaultSampleDuration           = representationInfo->trexDefaultSampleDurationForAudio;
        trexDefaultSampleFlags              = representationInfo->trexDefaultSampleFlagsForAudio;
        trexDefaultSampleSize               = representationInfo->trexDefaultSampleSizeForAudio;

        startTimeInMdhdTimescale =  (requestSeekCtx->currentRepresentationSeekContext->periodTimeInTimescaleUnits * mdhdTimescale ) / requestSeekCtx->currentRepresentationSeekContext->timescale;
    }
    else if (requestSeekCtx->currentRepresentationSeekContext == &requestSeekCtx->video) {
        fourcc            = representationInfo->fourccForVideo;
        codecSpecificAddr = representationInfo->codecSpecificAddrForVideo;
        codecSpecificSize = representationInfo->codecSpecificSizeForVideo;
        trackId           = representationInfo->trackIdForVideo;

        mdhdTimescale                       = representationInfo->mdhdTimescaleForVideo;
        trexDefaultSampleDescriptionIndex   = representationInfo->trexDefaultSampleDescriptionIndexForVideo;
        trexDefaultSampleDuration           = representationInfo->trexDefaultSampleDurationForVideo;
        trexDefaultSampleFlags              = representationInfo->trexDefaultSampleFlagsForVideo;
        trexDefaultSampleSize               = representationInfo->trexDefaultSampleSizeForVideo;

        startTimeInMdhdTimescale =  (requestSeekCtx->currentRepresentationSeekContext->periodTimeInTimescaleUnits * mdhdTimescale ) / requestSeekCtx->currentRepresentationSeekContext->timescale;
    }
    else {
        BDBG_ERR(("%s: Unable to determine media type of segment!", __FUNCTION__));
        goto error;
    }

    /*-----------------------------------------------------------------------------------
     *  Parse the downloaded segment in the Segment Buffer to get the start time from the
     *  "sidx" (Segment Index) box.  If the start time is there, use it, otherwise just
     *  use the Period Time that we already have.
     *---------------------------------------------------------------------------------*/
    if (B_PlaybackIp_MpegDashParseSegment(segmentBuffer, &startTimeFromSidxBox, &timescaleFromSidxBox)) {
        startTimeInMdhdTimescale = (startTimeFromSidxBox * mdhdTimescale )  / timescaleFromSidxBox;
    }

    /*-----------------------------------------------------------------------------------
     *  Okay, now attach a bfile file descriptor to the Segment Buffer, then create
     *  the various boxes needed for feeding fragmented MP4 files into NEXUS_Playpump.
     *---------------------------------------------------------------------------------*/
    fout = bfile_segment_buffer_random_write_attach(segmentBuffer);
    fout->seek(fout, 0, SEEK_SET);

    mp4_box_offset = start_mp4_box(fout, "bmp4");

    PRINTMSG_DOWNLOAD(("%s:%d: Creating bhed box... timescale: %lu start_time: %u fourcc: "BMEDIA_FOURCC_FORMAT, __FUNCTION__, __LINE__,
                             mdhdTimescale, (uint32_t)startTimeInMdhdTimescale, BMEDIA_FOURCC_ARG(fourcc)));
    write_bhed_box(    fout, mdhdTimescale,           startTimeInMdhdTimescale, fourcc);

    PRINTMSG_DOWNLOAD(("%s:%d: Creating bdat box... data: %p data_len: %u", __FUNCTION__, __LINE__,
                       (void *)codecSpecificAddr, (unsigned)codecSpecificSize ));
    write_bdat_box(fout,   codecSpecificAddr, codecSpecificSize);

    PRINTMSG_DOWNLOAD(("%s:%d: Creating trex box... track_ID: %lu, default_sample_description_index:%lu  default_sample_duration:%lu  default_sample_size:%lu  default_sample_flags:%lu",
                       __FUNCTION__, __LINE__,
                          trackId, trexDefaultSampleDescriptionIndex, trexDefaultSampleDuration, trexDefaultSampleFlags, trexDefaultSampleSize));
    write_trex_box( fout, trackId, trexDefaultSampleDescriptionIndex, trexDefaultSampleDuration, trexDefaultSampleFlags, trexDefaultSampleSize);

    /*---------------------------------------------------------------
     *  We've just written the (physically) last box, so do a sanity
     *  check and make sure the file is positioned just past the end
     *  of the space that we allocated for the boxes.
     *-------------------------------------------------------------*/
    {
        int current = fout->seek(fout, 0, SEEK_CUR);
        BDBG_ASSERT(current == segmentBuffer->mp4BoxPrefixSize);
    }

    /*---------------------------------------------------------------------
     *  Don't forget to seek to the end of the downloaded segment before
     *  finishing the "bmp4" box (to make sure everything gets included
     *  in the "bmp4" box).
     *-------------------------------------------------------------------*/
    fout->seek(fout, 0, SEEK_END);

    finish_mp4_box(fout, mp4_box_offset);

    bfile_segment_buffer_random_write_detach(fout);
    fout = NULL;

    return true;  /* normal success */

    /*---------------------------------------------------------------------
     *  Error handler.
     *-------------------------------------------------------------------*/
error:
    /*  if (fout) {
     *      bfile_segment_buffer_random_write_detach(fout);
     *  }
     */

    return false;
}


/*****************************************************************************
 * Re-Download Thread - For dynamic MPDs (which can be updated
 * on-the-fly, this thread periodically re-downloads the current
 * MPD to process any changes that might have been made since
 * the previous download.
 *****************************************************************************/
void
B_PlaybackIp_MpegDashMpdReDownloadThread(
    void *data
    )
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)data;
    MpegDashSessionState  * mpegDashSession      = playback_ip->mpegDashSessionState;
    char                  * myFuncName   = "ReDownloadThread";


#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: Started", myFuncName));
#endif
    while (true) {


        /*------------------------------------------------------------
         *  Just a skeleton for now... more to come later.
         *------------------------------------------------------------*/
        BKNI_Sleep(1000);
        BDBG_MSG(("%s: I am running!", myFuncName));

        /* Normal "End of Presentation". */
        if (mpegDashSession->mpegDashPlaybackThreadDone) {
            PRINTMSG_DOWNLOAD(("%s: MPEG-DASH Playback thread is done, so stopping the MPEG-DASH MPD ReDownload thread", myFuncName));

            /* downloadState = B_PlaybackMpegDashDownloadState_eExit; */



            /* Move this to the "exit" state handler. */
            mpegDashSession->mpegDashMpdReDownloadThreadDone = true;
            break;  /* GARYWASHERE:  TEMPORARY*/

        }
    }
    BDBG_MSG(("%s: Done", myFuncName));

    mpegDashSession->mpegDashMpdReDownloadThreadDone = true;
    return;
}


/*****************************************************************************
 * Download Thread - Waits for an available SegmentBuffer, then downloads
 * the next segment into it.
 *****************************************************************************/
void
B_PlaybackIp_MpegDashMediaSegmentDownloadThread(void *data)
{
    B_PlaybackIpHandle      playback_ip          = (B_PlaybackIpHandle)data;
    MpegDashSessionState  * mpegDashSession      = playback_ip->mpegDashSessionState;
    MpegDashMpdInfo       * mpdInfo              = mpegDashSession->mpdInfo;
    MpegDashSegmentBuffer * segmentBuffer        = NULL;
    MediaFileSegmentInfo  * mediaFileSegmentInfo = NULL;
    BERR_Code               rc;
    B_PlaybackIpError       pbIpErrCode;
    unsigned                segmentErrorCount = 0;

    typedef enum B_PlaybackMpegDashDownloadState {
        B_PlaybackMpegDashDownloadState_eInit,
        B_PlaybackMpegDashDownloadState_eStart,
        B_PlaybackMpegDashDownloadState_eSegBufWait,
        B_PlaybackMpegDashDownloadState_eAdvanceSeek,
        B_PlaybackMpegDashDownloadState_eCheckProbeInfo,
        B_PlaybackMpegDashDownloadState_eCreateMFSI,
        B_PlaybackMpegDashDownloadState_eStartDownload,
        B_PlaybackMpegDashDownloadState_eCheckDownload,
        B_PlaybackMpegDashDownloadState_eFinishDownload,
        B_PlaybackMpegDashDownloadState_eUpdateRepr,
        B_PlaybackMpegDashDownloadState_eTrickModeSuspend,
        B_PlaybackMpegDashDownloadState_eReset,
        B_PlaybackMpegDashDownloadState_eExit,
    } B_PlaybackMpegDashDownloadState;

    B_PlaybackMpegDashDownloadState  downloadState = B_PlaybackMpegDashDownloadState_eInit;

    bool    serverClosed   = true;
    char  * myFuncName   = "DownloadThread";


    PRINTMSG_DOWNLOAD(("%s: Started", myFuncName));

    /* Set our wake event so we'll take a pass through the state
     * machine.  */
    PRINTMSG_DOWNLOAD((":%d %s(): Setting downloadThreadWakeEvent.", __LINE__, myFuncName));
    BKNI_SetEvent(mpegDashSession->downloadThreadWakeEvent);

    /*------------------------------------------------------------
     *  Top of state processing loop.
     *------------------------------------------------------------*/
    while (true) {
        PRINTMSG_DOWNLOAD(("%s:%d : At top of main Download loop, waiting for wakeup... ", myFuncName, __LINE__ ));

        /*------------------------------------------------------------
         *  Wait here for something to happen.
         *------------------------------------------------------------*/
        rc = BKNI_WaitForEvent(mpegDashSession->downloadThreadWakeEvent, MPEG_DASH_EVENT_TIMEOUT_MSEC );
        if (rc == BERR_SUCCESS) {
            PRINTMSG_DOWNLOAD(("%s: Got wakeup event, checking current state.", myFuncName));
        }
        else if (rc == BERR_TIMEOUT) {
            PRINTMSG_DOWNLOAD(("%s: Failsafe wakeup, checking current state.", myFuncName));
        }
        else  {
            BDBG_ERR(("%s: Failed to wait for downloadThreadWakeEvent, rc = %d, giving up.", myFuncName, rc));
            rc = BERR_TRACE(rc);
            goto error;
        }

        /*------------------------------------------------------------
         *  Do some checks here for things that don't depend on the
         *  curent state.
         *------------------------------------------------------------*/

        /* Normal shutdown. */
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
            PRINTMSG_DOWNLOAD(("%s: breaking out of MPEG-DASH Media Segment Download loop due to state (%d) change", myFuncName, playback_ip->playback_state));
            downloadState = B_PlaybackMpegDashDownloadState_eExit;
        }

        /* Normal "End of Presentation". */
        if (mpegDashSession->mpegDashPlaybackThreadDone) {
            PRINTMSG_DOWNLOAD(("%s: MPEG-DASH Playback thread is done, so stopping the MPEG-DASH Segment Download thread", myFuncName));
            downloadState = B_PlaybackMpegDashDownloadState_eExit;
        }

        /* Entering trick mode (seek request). */
        if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode) {

            PRINTMSG_DOWNLOAD(("%s: App is trying for a trickmode command, pause this thread", myFuncName));
            /* signal the control thread that we have paused and thus it can continue w/ setting up the trickmode operation */
            BKNI_ResetEvent(mpegDashSession->segDownloadThreadPauseDoneEvent);
            BKNI_SetEvent(mpegDashSession->segDownloadThreadPausedEvent);

            downloadState = B_PlaybackMpegDashDownloadState_eTrickModeSuspend;
        }

        /*------------------------------------------------------------
         *  State: eInit (transient): This state only occurs once... at
         *  program startup.,
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eInit) {
            PRINTMSG_DOWNLOAD(("%s:%d : Handling Init state...", myFuncName, __LINE__ ));
            downloadState = B_PlaybackMpegDashDownloadState_eStart;
        }

        /*------------------------------------------------------------
         *  State: eStart (transient): Start the process of downloading
         *  a segment file.
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eStart) {
            PRINTMSG_DOWNLOAD(("%s:%d : Handling Start state...", myFuncName, __LINE__ ));
            downloadState = B_PlaybackMpegDashDownloadState_eSegBufWait;
        }

        /*------------------------------------------------------------
         *  State: eSegBufWait (stable): Wait in this state until we can
         *  allocate a SegmentBuffer to hold the segment to be
         *  downloaded.
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eSegBufWait) {
            PRINTMSG_DOWNLOAD(("%s:%d : Handling SegBufWait state...", myFuncName, __LINE__ ));

            /* I don't think we should ever have a SegmentBuffer here, but
             * if we do, just free it up and get a new one.  */
            if (segmentBuffer) {
                B_PlaybackIp_MpegDashSegmentBufferFree(playback_ip, segmentBuffer); segmentBuffer = NULL;
            }

            /* Try to find a SegmentBuffer... if we find one, move to the
             * next state, otherwise just stay here and try again later. */
            segmentBuffer = B_PlaybackIp_MpegDashSegmentBufferAlloc(playback_ip);
            if (segmentBuffer) {
                downloadState = B_PlaybackMpegDashDownloadState_eUpdateRepr;
            }
        }

        /*------------------------------------------------------------
         *  State: eUpdateRepr (transient): Update the current
         *  Representation according to the measured network bandwidth.
         *  This will choose content which matches the speed of the
         *  network connection.
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eUpdateRepr) {
            PRINTMSG_DOWNLOAD(("%s:%d : Handling UpdateRepr state...", myFuncName, __LINE__ ));

            /* This is where we do the adaptive Representation change based on
             * the measured bandwidth.  */
            if (!B_PlaybackIp_MpegDashCheckForRepresentationChange(playback_ip, &mpegDashSession->requestSeekCtx)) {
                BDBG_ERR(("%s: Failed to do adaptive Representation change!", myFuncName));
            }
            downloadState = B_PlaybackMpegDashDownloadState_eAdvanceSeek;
        }

        /*------------------------------------------------------------
         *  State: eAdvanceSeek (stable): Stay in this state until we
         *  have a valid seek context. An invalid seek context can occur
         *  during a "dynamic" presentation when we are trying to read a
         *  segment that doesn't exist yet.   In that case, we'll sit in
         *  this state until it's time for the next segment to exist.
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eAdvanceSeek) {
            PRINTMSG_DOWNLOAD(("%s:%d : Handling AdvanceSeek state...", myFuncName, __LINE__ ));

            /* If the seek status is already valid (like at the first
             * segment of a presentation, then we can skip the advance and
             * just go to the next step.  */
            if (mpegDashSession->requestSeekCtx.currentRepresentationSeekContext->status == B_PlaybackMpegDashSeekStatus_eValid) {

                downloadState = B_PlaybackMpegDashDownloadState_eCheckProbeInfo;    /* Go to next state. */
            }
            else {
                pbIpErrCode = B_PlaybackIp_MpegDashSeekAdvanceBySegment(playback_ip, &mpegDashSession->requestSeekCtx);
                if (pbIpErrCode == B_ERROR_SUCCESS) {
                    downloadState = B_PlaybackMpegDashDownloadState_eCheckProbeInfo;    /* Go to next state. */
                }
                else if (pbIpErrCode == B_ERROR_INVALID_PARAMETER) {    /* Seek past end of presentation.  */
                    if (mpdInfo->isDynamic) {
                        ;   /* Another EOF, just stay in this state and try again later.  */
                    }
                    else {
                        mpegDashSession->downloadedAllSegmentsInCurrentRepresentation = true;
                    }
                }
                else {  /* Fatal Error from ...AdvanceBySegment() */
                    BDBG_ERR(("%s: Can't seek to next segment in Representation", myFuncName));
                    pbIpErrCode = BERR_TRACE(pbIpErrCode);
                    goto error;
                }
            }
        }

        /*------------------------------------------------------------
         *  State: eCheckProbeInfo (stable): If the Representation for
         *  the current seek does not have valid probe info, then probe
         *  the initialization segment.
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eCheckProbeInfo) {

            MpegDashRepresentationSeekContext   * representationSeekCtx = mpegDashSession->requestSeekCtx.currentRepresentationSeekContext;
            MpegDashRepresentationInfo          * representationInfo    = representationSeekCtx->representationInfo;

            PRINTMSG_DOWNLOAD(("%s:%d : Handling CheckProbeInfo state...", myFuncName, __LINE__ ));

            if (representationInfo->probeInfoIsValid) {
                downloadState = B_PlaybackMpegDashDownloadState_eCreateMFSI;
            }
            else {
                bool rc;

                rc =B_PlaybackIp_MpegDashProbeForRepresentationStreamInfo(playback_ip, representationSeekCtx, segmentBuffer);
                if (!rc) {   /* Error... Stay in this state until we get probe info. */
                    BDBG_ERR(("%s: Failed to probe Representation stream info.", myFuncName));
                }
                else {      /* Success, transition to next state. */

                    /* Try to add slave pids for the new Representation. */
                    if (!B_PlaybackIp_MpegDashAddSlavePidChannel(playback_ip, NEXUS_PidType_eVideo, playback_ip->psi.videoPid)) {
                        BERR_TRACE(B_ERROR_UNKNOWN);
                        goto error;
                    }

                    if (!B_PlaybackIp_MpegDashAddSlavePidChannel(playback_ip, NEXUS_PidType_eAudio, playback_ip->psi.audioPid)) {
                        BERR_TRACE(B_ERROR_UNKNOWN);
                        goto error;
                    }

                    downloadState = B_PlaybackMpegDashDownloadState_eCreateMFSI;
                }
            }
        }

        /*------------------------------------------------------------
         *  State: eCreateMFSI (transient): Create a
         *  MediaFileSegmentInfo structure that describes the next
         *  segment to be downloaded.,
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eCreateMFSI) {
            PRINTMSG_DOWNLOAD(("%s:%d : Handling CreateMFSI state...", myFuncName, __LINE__ ));
            if (mediaFileSegmentInfo == NULL) {

                mediaFileSegmentInfo = B_PlaybackIp_MpegDashCreateMediaFileSegmentInfoFromSeek(
                                                playback_ip,
                                                mpegDashSession->requestSeekCtx.currentRepresentationSeekContext,
                                                MPEG_DASH_URL_KIND_MEDIA);
                if (mediaFileSegmentInfo == NULL) {
                    BDBG_ERR(("%s: Failed to build Media File Segment Info.", myFuncName));
                    pbIpErrCode = BERR_TRACE(B_ERROR_UNKNOWN);
                    goto error;
                }
            }
            downloadState = B_PlaybackMpegDashDownloadState_eStartDownload;
        }

        /*------------------------------------------------------------
         *  State: eStartDownload (stable): Start the network download
         *  by sending an HTTP GET request to the server.
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eStartDownload) {
            PRINTMSG_DOWNLOAD(("%s : %d : Handling StartDownload state...", myFuncName, __LINE__ ));

            /* Allocate space in the segment buffer to hold MP4 boxes that
             * we'll fill in later. */
            if (mpegDashSession->mpegType == NEXUS_TransportType_eMp4Fragment) {
                if (!B_PlaybackIp_MpegDashAllocateSegmentBoxPrefix( segmentBuffer, &mpegDashSession->requestSeekCtx )) {
                    goto error;
                }
            }

            PRINTMSG_SUMMARY(( "%s:%d: Started  download: %s", myFuncName, __LINE__, mediaFileSegmentInfo->uri));
            PRINTMSG_DOWNLOAD(("%s:%d: Started  download: %s", myFuncName, __LINE__, mediaFileSegmentInfo->uri));

            if (B_PlaybackIp_MpegDashMediaSegmentDownloadStart(
                             playback_ip,
                             &mpegDashSession->downloadContext,
                             mediaFileSegmentInfo,
                             &playback_ip->socketState.fd,
                             segmentBuffer) == false) {

                BDBG_ERR(("%s:%d ERROR: Segment Download failed for \"%s\"", myFuncName, __LINE__,mediaFileSegmentInfo->absoluteUri));

                /* GARYWASHERE: need to handle this situation. Probably should give up after several failures. */
                /*------------------------------------------------------------
                 *  This can get complicated... If we get something like a 404
                 *  (file not found) status code, there's a few different ways
                 *  that we might want to handle it.
                 *
                 *  Case 1: Static MPD
                 *  Perhaps do limited retries, then skip the segment and
                 *  continue.  If the segment is really missing, it's probably
                 *  not going to magically show up.
                 *
                 *  Case 2: Dynamic MPD - Beginning of Timeshift Buffer
                 *  Limited retries, then skip segment and continue.  Segment is
                 *  probably missing because it expired and the server has
                 *  deleted it.
                 *
                 *  Case 3: Dynamic MPD - End of Timeshift Buffer
                 *  We are probably trying to read a segment that hasn't been
                 *  written yet. If that's the case, then skipping this segment
                 *  won't help because the next segment won't be there either.
                 *  So we should probably just wait for a while (a segment
                 *  duration?) then try the same segment again.
                 *------------------------------------------------------------*/

                /* Switch between audio and video AdaptationSets if required. */
                pbIpErrCode = B_PlaybackIp_MpegDashSeekSwitchAdaptationSet(playback_ip, &mpegDashSession->requestSeekCtx);
                if (pbIpErrCode != B_ERROR_SUCCESS) {
                    BDBG_ERR(("%s: Failed to switch adaptation sets!", myFuncName));
                    pbIpErrCode = BERR_TRACE(pbIpErrCode);
                    goto error;
                }

                downloadState = B_PlaybackMpegDashDownloadState_eReset;
            }
            else {  /* Download was started successfully. */
                PRINTMSG_DOWNLOAD(("%s:%d : Download is now in progress", myFuncName, __LINE__ ));
                downloadState = B_PlaybackMpegDashDownloadState_eCheckDownload;
            }
        }

        /*------------------------------------------------------------
         *  State: eCheckDownload (stable): Stay in this state until the
         *  download is completed.
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eCheckDownload) {
            PRINTMSG_DOWNLOAD(("%s:%d : Handling CheckDownload state...", myFuncName, __LINE__ ));

            /* now download the actual media segment */
            serverClosed = false;
            PRINTMSG_DOWNLOAD(("%s:%d : Checking for Download complete", myFuncName, __LINE__ ));
            if ( B_PlaybackIp_MpegDashMediaSegmentDownloadFinish(playback_ip, &mpegDashSession->downloadContext, &serverClosed) != true ) {
                /* coverity[var_deref_op: FALSE] */
                BDBG_ERR(("%s:%d ERROR: Segment Download failed for \"%s\"", myFuncName, __LINE__,mediaFileSegmentInfo->absoluteUri));

                downloadState = B_PlaybackMpegDashDownloadState_eReset;

                if (++segmentErrorCount > 3) {
                    BDBG_ERR(("%s:%d ERROR: Exceeded retry count, giving up.", myFuncName, __LINE__));
                    downloadState = B_PlaybackMpegDashDownloadState_eExit;
                }
            }
            else {  /* Download completed successfully. */

                PRINTMSG_SUMMARY(( "%s:%d: Finished download: %s", myFuncName, __LINE__, mediaFileSegmentInfo->uri));
                PRINTMSG_DOWNLOAD(("%s:%d: Finished download: %s", myFuncName, __LINE__, mediaFileSegmentInfo->uri));

                segmentErrorCount = 0;

                /* Check if we downloaded the complete media segment. If not,
                 * then the segment is too large for us to handle (for now at
                 * least).  In that case, just go ahead and try to continue
                 * with the truncated segment.  */
                if (!serverClosed) {
                    if (segmentBuffer->bufferDepth >= segmentBuffer->bufferSize) {
                        BDBG_ERR(("%s: Didn't download all bytes of current media segment: bytesDownloaded %d", myFuncName, (int)segmentBuffer->bufferDepth));
                        BDBG_ERR(("%s:%d: Segment size exceeded maximum! Trying to continue.", myFuncName, __LINE__));
                    }
                }

                downloadState = B_PlaybackMpegDashDownloadState_eFinishDownload;
            }
        }

        /*------------------------------------------------------------
         *  State: eFinishDownload (transient): Do some things that need
         *  to be done after the download has completed.
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eFinishDownload) {
            PRINTMSG_DOWNLOAD(("%s:%d : Handling FinishDownload state...", myFuncName, __LINE__ ));

            /* Add this bandwidth sample to the bandwidth accumulator */
            B_PlaybackIp_MpegDashAddBandwidthSample(&mpegDashSession->bandwidthContext, mpegDashSession->downloadContext.elapsedTimeInMs, mpegDashSession->downloadContext.totalBytesReadSoFar);

            /* Prefix the segment with the MP4 boxes needed for playpump. */
            if (mpegDashSession->mpegType == NEXUS_TransportType_eMp4Fragment) {
                if (!B_PlaybackIp_MpegDashBuildSegmentBoxPrefix( segmentBuffer, &mpegDashSession->requestSeekCtx ) ) {
                    goto error;
                }
            }

            PRINTMSG_DOWNLOAD(("%s: Informing MPEG-DASH playback thread that buffer %p: depth %d is filled , current network bandwidth %lu Kbps",
                               myFuncName, (void *)segmentBuffer, (int)segmentBuffer->bufferDepth,
                               B_PlaybackIp_MpegDashGetCurrentBandwidth(&mpegDashSession->bandwidthContext) ));

            /* Hand off the segment buffer to the Playback thread. */
            {
                BKNI_AcquireMutex(segmentBuffer->lock);   /* segment buffer locked... */

                /* coverity[var_deref_op: FALSE] */
                if (mediaFileSegmentInfo->markedDiscontinuity) {
                    segmentBuffer->markedDiscontinuity = true;
                }
                segmentBuffer->hasAudio = mediaFileSegmentInfo->hasAudio;
                segmentBuffer->hasVideo = mediaFileSegmentInfo->hasVideo;

                /* inform MPEG-DASH playback thread that buffer is filled w/ the media segment */
                segmentBuffer->filled = true;   /* This hands off the segment buffer to the Playback thread */

                BKNI_ReleaseMutex(segmentBuffer->lock); /* segment buffer unlocked.  */
            }
            segmentBuffer = NULL;   /* The segment buffer no longer belongs to us... */

            PRINTMSG_PLAYBACK(("%s:%d: Setting playbackThreadWakeEvent to process filled SegmentBuffer.", myFuncName, __LINE__));
            BKNI_SetEvent(mpegDashSession->playbackThreadWakeEvent);  /* Wake up the playback thread.  */

            /* We're done with this MediaFileSegmentInfo, so get rid of it.  */
            B_PlaybackIp_MpegDashFreeMediaFileSegmentInfo(mediaFileSegmentInfo); mediaFileSegmentInfo = NULL;

            /* Switch between audio and video AdaptationSets if required. */
            pbIpErrCode = B_PlaybackIp_MpegDashSeekSwitchAdaptationSet(playback_ip, &mpegDashSession->requestSeekCtx);
            if (pbIpErrCode != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to switch adaptation sets!", myFuncName));
                pbIpErrCode = BERR_TRACE(pbIpErrCode);
                goto error;
            }

            /* Go back to the Start state to download the next segment. */
            downloadState = B_PlaybackMpegDashDownloadState_eStart;
            PRINTMSG_DOWNLOAD((":%d %s(): Setting downloadThreadWakeEvent.", __LINE__, myFuncName));
            BKNI_SetEvent(mpegDashSession->downloadThreadWakeEvent); /* Self-wake to process new state. */
        }

        /*------------------------------------------------------------
         *  State: eTrickModeSuspend (stable):
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eTrickModeSuspend)
        {
            PRINTMSG_DOWNLOAD(("%s:%d : Handling TrickModeSuspend state...", myFuncName, __LINE__ ));

            /* wait on signal from the control thread to indicate trickmode work is done */
            rc = BKNI_WaitForEvent(mpegDashSession->segDownloadThreadPauseDoneEvent, 100 * MPEG_DASH_EVENT_TIMEOUT_MSEC);
            BKNI_ResetEvent(mpegDashSession->segDownloadThreadPausedEvent);  /* Not in paused state anymore. */
            if (rc == BERR_TIMEOUT || rc!=0) {
                BDBG_ERR(("%s: EVENT %s: failed to receive event from control thread indicating trickmode completion", myFuncName, rc==BERR_TIMEOUT?"Timeout":"Error"));
                goto error;
            }

            PRINTMSG_DOWNLOAD(("%s:%d : Resuming from TrickModeSuspend ", myFuncName, __LINE__   ));
            downloadState = B_PlaybackMpegDashDownloadState_eReset;
        }

        /*------------------------------------------------------------
         *  State: eReset (transient):
         *  State: eExit  (transient):
         *------------------------------------------------------------*/
        if (downloadState == B_PlaybackMpegDashDownloadState_eReset ||
            downloadState == B_PlaybackMpegDashDownloadState_eExit)     {
            PRINTMSG_DOWNLOAD(("%s:%d : Handling Reset/Exit state...", myFuncName, __LINE__ ));

            if (segmentBuffer) {
                B_PlaybackIp_MpegDashSegmentBufferFree(playback_ip, segmentBuffer); segmentBuffer = NULL;
            }

            if (mediaFileSegmentInfo) {
                B_PlaybackIp_MpegDashFreeMediaFileSegmentInfo(mediaFileSegmentInfo); mediaFileSegmentInfo = NULL;
            }

            if (playback_ip->securityHandle) {
                playback_ip->netIo.close(playback_ip->securityHandle);
                playback_ip->securityHandle = NULL;
            }

            /* Make sure that the socket is closed. */
            if (mpegDashSession->downloadContext.socketFd != -1) {
                close(mpegDashSession->downloadContext.socketFd);
                mpegDashSession->downloadContext.socketFd = -1;
            }

            /* We're done cleaning things up.  If this is the eExit state,
             * then exit the state processing loop.  */
            if (downloadState == B_PlaybackMpegDashDownloadState_eExit ) {
                break;
            }

            /* Otherwise, this is the eReset state so just take it from
             * the top...*/
            downloadState = B_PlaybackMpegDashDownloadState_eStart;
        }
    }

    PRINTMSG_DOWNLOAD(("%s: Normal exit from state processing loop...", myFuncName));

error:
    if (segmentBuffer) {
        B_PlaybackIp_MpegDashSegmentBufferFree(playback_ip, segmentBuffer); segmentBuffer = NULL;
    }

    if (mediaFileSegmentInfo) {
        B_PlaybackIp_MpegDashFreeMediaFileSegmentInfo(mediaFileSegmentInfo); mediaFileSegmentInfo = NULL;
    }

    mpegDashSession->mpegDashSegmentDownloadThreadDone = true;
    PRINTMSG_DOWNLOAD(("%s: MPEG-DASH Media Segment Download thread is exiting...", myFuncName));
    return;
}


/*****************************************************************************
 * Check to see if the stream has finished playing through the Playpump and
 * decoders.
 *****************************************************************************/
bool
B_PlaybackIp_MpegDashEndOfStream(
    B_PlaybackIpHandle playback_ip
    )
{
    NEXUS_Error rc;
    NEXUS_PlaypumpStatus playpumpStatus;
    NEXUS_VideoDecoderStatus decoderStatus;

    /* TODO: can't sleep here as user may be trying to stop/close during this time and thus that would be stuck */
    /* instead, check if we have reached the end and return true or false here */
    /* caller should check for stop/stopping state */

    if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped || playback_ip->psi.liveChannel) {
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: %s (playback ip state %d), so return immediately", __FUNCTION__, playback_ip->psi.liveChannel ? "live channel":"app has issued stop", playback_ip->playback_state));
#endif
        return true;
    }

    /* Check to see if Playpump has emptied yet. */
    if (playback_ip->nexusHandles.playpump) {

        rc=NEXUS_Playpump_GetStatus(playback_ip->nexusHandles.playpump, &playpumpStatus);
        if (rc!=NEXUS_SUCCESS) {
            return false;       /* Shouldn't happen */
        }

        if(playpumpStatus.fifoDepth > 0) {
           BDBG_NUL(("%s:%d: playpumpStatus.fifoDepth=%u", __FUNCTION__, __LINE__, playpumpStatus.fifoDepth));
            return false;       /* Playpump still has data, not done yet. */
        }
    }

    /* Check to see if the Video Decoder is emptied. */
    if (playback_ip->nexusHandles.videoDecoder) {

        rc=NEXUS_VideoDecoder_GetStatus(playback_ip->nexusHandles.videoDecoder, &decoderStatus);
        if (rc!=NEXUS_SUCCESS) {
            return false;       /* Shouldn't happen */
        }
        if(decoderStatus.queueDepth > 0) {
            BDBG_NUL(("%s:%d: decoderStatus.queueDepth=%u", __FUNCTION__, __LINE__, decoderStatus.queueDepth));
            return false;       /* Video decoder still has data, not done yet. */
        }
    }

    /* GARYWASHERE: What about the Audio Decoder?? */

    return true;
}


/*****************************************************************************
 * Check to see if the stream has finished playing through the Playpump and
 * decoders.
 *****************************************************************************/
int B_PlaybackIp_MpegDashGetPlaypumpBuffer(
    B_PlaybackIpHandle playback_ip,
    NEXUS_PlaypumpHandle playpump,
    unsigned int size
    )
{
    NEXUS_Error rc;
    for(;;) {
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode) {
            goto error;
        }

        if (NEXUS_Playpump_GetBuffer(playpump, &playback_ip->buffer, &playback_ip->buffer_size)) {
            BDBG_ERR(("Returned error from NEXUS_Playpump_GetBuffer()!"));
            goto error;
        }

        if (playback_ip->buffer_size == 0) {
            NEXUS_PlaypumpStatus ppStatus;
            /* bplaypump_flush(playback_ip->nexusHandles.playpump); */
            rc = NEXUS_Playpump_GetStatus(playpump, &ppStatus);
            if (!rc) BDBG_MSG(("Returned 0 buffer size from GetBuffer()!, fifo dep %d, size %d, desc dep %d sz %d", (int)ppStatus.fifoDepth, (int)ppStatus.fifoSize, (int)ppStatus.descFifoDepth, (int)ppStatus.descFifoSize));
            BKNI_Sleep(200);
            continue;
        }
        else if(playback_ip->buffer_size >= size) {
            /* constrain the amount of data we're going to read from the socket */
            playback_ip->buffer_size = size;
            BDBG_MSG(("%s: got buffer of size %d from the playpump\n", __FUNCTION__, size));
            break;
        }
        BDBG_MSG_FLOW(("skip buffer %d", playback_ip->buffer_size));
        /* release buffer unused, it's too small */
        if(NEXUS_Playpump_ReadComplete(playpump, playback_ip->buffer_size, 0)) {
            BDBG_ERR(("Returned error from NEXUS_Playpump_ReadComplete()!"));
            BKNI_Sleep(1);
            continue;
        }
    }
    return (playback_ip->buffer_size);
error:
    return -1;
}


/*****************************************************************************
 * Playback Thread - Waits for a SegmentBuffer to be filled by the Download
 * Thread, then feeds the segment to NEXUS_Playpump.
 *****************************************************************************/
void
B_PlaybackIp_MpegDashPlaybackThread(void *data)
{
    B_PlaybackIpHandle      playback_ip;
    MpegDashSessionState  * mpegDashSession;
    MpegDashMpdInfo       * mpdInfo;
    MpegDashSegmentBuffer * segmentBuffer = NULL;


    typedef enum B_PlaybackMpegDashPlaybackState {
        B_PlaybackMpegDashPlaybackState_eInit,
        B_PlaybackMpegDashPlaybackState_eStart,
        B_PlaybackMpegDashPlaybackState_eSegBufWait,
        B_PlaybackMpegDashPlaybackState_ePlaypumpBegin,
        B_PlaybackMpegDashPlaybackState_ePlaypumpFeed,
        B_PlaybackMpegDashPlaybackState_eSegBufFree,
        B_PlaybackMpegDashPlaybackState_eTrickModeSuspend,
        B_PlaybackMpegDashPlaybackState_eReset,
        B_PlaybackMpegDashPlaybackState_eExit,
    } B_PlaybackMpegDashPlaybackState;

    B_PlaybackMpegDashPlaybackState  playbackState = B_PlaybackMpegDashPlaybackState_eInit;


    int i;
    B_ThreadSettings    myThreadSettings;
    const char        * downloadThreadName      = "MpegDashMediaSegmentDownloadThread";
    const char        * mpdRedownloadThreadName = "MpegDashMpdReDownloadThread";

    ssize_t segmentBytesFed = 0;
    ssize_t rc = -1;
    off_t totalBytesFed = 0;
    NEXUS_PlaypumpSettings nSettings;
    static int fileNameSuffix = 0;
    char recordFileName[32];
    FILE *fclear = NULL;
    char * myFuncName = "PlaybackThread";
    unsigned long maxAudioFeedSize;
    unsigned long maxVideoFeedSize;


    /*------------------------------------------------------------
     *  Entry point.
     *------------------------------------------------------------*/
    BDBG_ENTER(B_PlaybackIp_MpegDashPlaybackThread);

    playback_ip     = (B_PlaybackIpHandle)data;
    mpegDashSession = playback_ip->mpegDashSessionState;
    mpdInfo         = mpegDashSession->mpdInfo;

    /*------------------------------------------------------------
     *  Adjust network timeout as required.
     *------------------------------------------------------------*/
    if (playback_ip->settings.networkTimeout) {
        if (playback_ip->settings.networkTimeout > (HTTP_SELECT_TIMEOUT/10))
            playback_ip->networkTimeout = HTTP_SELECT_TIMEOUT/10;
        else
            playback_ip->networkTimeout = playback_ip->settings.networkTimeout;
    }
    else {
        playback_ip->networkTimeout = HTTP_SELECT_TIMEOUT/10;
    }
    PRINTMSG_PLAYBACK(("%s: Starting (network timeout %d secs)", myFuncName, playback_ip->networkTimeout));

    /*------------------------------------------------------------
     *  Start the Download thread to download the segments that we
     *  are going to feed to the playpump.
     *------------------------------------------------------------*/
    B_Thread_GetDefaultSettings(&myThreadSettings);
    mpegDashSession->mpegDashSegmentDownloadThread =
                    B_Thread_Create(downloadThreadName,
                                    B_PlaybackIp_MpegDashMediaSegmentDownloadThread,
                                    (void *)playback_ip,
                                    &myThreadSettings);
    if (NULL == mpegDashSession->mpegDashSegmentDownloadThread) {
        BDBG_ERR(("%s: Failed to create the %s thread for MPEG-DASH protocol\n", myFuncName, downloadThreadName));
        goto error;
    }
    PRINTMSG_PLAYBACK(("%s: Created the %s for MPEG-DASH protocol", myFuncName, downloadThreadName));


    /*------------------------------------------------------------
     *  Determine the max chunk size for feeding each of the
     *  playpumps.  We'll use a percentage of the space allocated to
     *  the playpump.
     *------------------------------------------------------------*/
    {
        NEXUS_PlaypumpStatus playpumpStatus;
        NEXUS_PlaypumpHandle audioPlaypumpHandle = B_PlaybackIp_MpegDashGetPlaypumpHandle(playback_ip,NEXUS_PidType_eAudio);
        NEXUS_PlaypumpHandle videoPlaypumpHandle = B_PlaybackIp_MpegDashGetPlaypumpHandle(playback_ip,NEXUS_PidType_eVideo);

        maxAudioFeedSize = 64 * 1024;   /* Set failsafe value in case something goes wrong. */
        if (audioPlaypumpHandle) {
            rc = NEXUS_Playpump_GetStatus(audioPlaypumpHandle, &playpumpStatus);
            if (rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s:%d: NEXUS_Playpump_GetStatus failed! rc=%d", myFuncName, __LINE__, (int)rc));
            }
            else {
                maxAudioFeedSize = playpumpStatus.fifoSize / 16;
            }
        }

        maxVideoFeedSize = 512 * 1024;   /* Set failsafe value in case something goes wrong. */
        if (videoPlaypumpHandle) {
            rc = NEXUS_Playpump_GetStatus(videoPlaypumpHandle, &playpumpStatus);
            if (rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s:%d: NEXUS_Playpump_GetStatus failed! rc=%d", myFuncName, __LINE__, (int)rc));
            }
            else {
                maxVideoFeedSize = playpumpStatus.fifoSize / 16;
            }
        }
    }

    /*------------------------------------------------------------
     *  Set up our callback so that Playpump can notify when it has
     *  room for more data.
     *------------------------------------------------------------*/
    if (playback_ip->nexusHandles.playpump) {
        NEXUS_Playpump_GetSettings(playback_ip->nexusHandles.playpump, &nSettings);
        nSettings.dataCallback.callback = B_PlaybackIp_MpegDashPlaypumpCallback;
        nSettings.dataCallback.context = playback_ip;
        if (NEXUS_Playpump_SetSettings(playback_ip->nexusHandles.playpump, &nSettings)) {
            BDBG_ERR(("%s:%d Nexus Error: %d\n", myFuncName, __LINE__, (int)rc));
            goto error;
        }
    }
    else {
        BDBG_ERR(("%s: playback_ip->nexusHandles.playpump is NULL", myFuncName));
        goto error;
    }

    if (playback_ip->nexusHandles.playpump2) {
        NEXUS_Playpump_GetSettings(playback_ip->nexusHandles.playpump2, &nSettings);
        nSettings.dataCallback.callback = B_PlaybackIp_MpegDashPlaypumpCallback;
        nSettings.dataCallback.context = playback_ip;
        if (NEXUS_Playpump_SetSettings(playback_ip->nexusHandles.playpump2, &nSettings)) {
            BDBG_ERR(("%s:%d Nexus Error: %d\n", myFuncName, __LINE__, (int)rc));
            goto error;
        }
    }

    /*------------------------------------------------------------
     *  Set up for writing the Playpump data to a file (for
     *  debugging).
     *------------------------------------------------------------*/
    if (playback_ip->enableRecording) {
        memset(recordFileName, 0, sizeof(recordFileName));
        snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/mpegDash_rec%d_0.ts", fileNameSuffix++);
        fclear = fopen(recordFileName, "w+b");
        if (fclear == NULL) perror("fopen failed: ");
    }

    /*------------------------------------------------------------
     *  Wait until the Playpump is started.
     *------------------------------------------------------------*/
    if (B_PlaybackIp_UtilsWaitForPlaypumpDecoderSetup(playback_ip))
        goto error;


    /*------------------------------------------------------------
     *  For Dynamic MPDs, we need to start a "redownload" thread to
     *  periodically redownload the MPD (since it can change
     *  on-the-fly).
     *------------------------------------------------------------*/
    if (mpdInfo->isDynamic) {
        /* The MPD can change "on the fly", so we'll need to re-read
         * it periodically.  Start the ReDownload thread to do that.  */
        mpegDashSession->mpegDashMpdReDownloadThread =
                    B_Thread_Create(mpdRedownloadThreadName,
                                    B_PlaybackIp_MpegDashMpdReDownloadThread,
                                    (void *)playback_ip,
                                    &myThreadSettings);

        if (NULL == mpegDashSession->mpegDashMpdReDownloadThread) {
            BDBG_ERR(("%s: Failed to create the %s thread for MPEG-DASH protocol\n", myFuncName, mpdRedownloadThreadName));
            goto error;
        }

        PRINTMSG_PLAYBACK(("%s: Created the %s for MPEG-DASH protocol", myFuncName, mpdRedownloadThreadName));
    }

    /* Set our wake event so we'll take a pass through the state
     * machine.  */
    BKNI_SetEvent(mpegDashSession->playbackThreadWakeEvent);
    PRINTMSG_PLAYBACK(("%s:%d: Setting playbackThreadWakeEvent to start the state machine.", myFuncName, __LINE__));

    /*------------------------------------------------------------
     *  Top of state processing loop.
     *------------------------------------------------------------*/
    while (true) {
        PRINTMSG_PLAYBACK(("%s:%d : At top of main Playback loop, waiting for wakeup... ", myFuncName, __LINE__ ));

        /*------------------------------------------------------------
         *  Wait here for something to happen.
         *------------------------------------------------------------*/
        rc = BKNI_WaitForEvent(mpegDashSession->playbackThreadWakeEvent, MPEG_DASH_EVENT_TIMEOUT_MSEC );
        if (rc == BERR_SUCCESS) {
            PRINTMSG_PLAYBACK(("%s: Got wakeup event, checking current state.", myFuncName));
        }
        else if (rc == BERR_TIMEOUT) {
            PRINTMSG_PLAYBACK(("%s: Failsafe wakeup, checking current state.", myFuncName));
        }
        else  {
            BDBG_ERR(("%s: Failed to wait for playbackThreadWakeEvent, rc = %d, giving up.", myFuncName, (int)rc));
            rc = BERR_TRACE(rc);
            goto error;
        }

        /*------------------------------------------------------------
         *  Do some checks here for things that don't depend on the
         *  curent state.
         *------------------------------------------------------------*/

        /* Normal shutdown. */
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
            PRINTMSG_PLAYBACK(("%s: breaking out of MPEG-DASH Playback loop due to state (%d) change", myFuncName, playback_ip->playback_state));
            playbackState = B_PlaybackMpegDashPlaybackState_eExit;
        }

            /* Maybe this isn't needed???  GARYWASHERE */
#if 0  /* ==================== GARYWASHERE - Start of Original Code ==================== */
        /* Normal "End of Presentation". */
        if (mpegDashSession->mpegDashPlaybackThreadDone) {
            PRINTMSG_PLAYBACK(("%s: MPEG-DASH Playback thread is done, so stopping the MPEG-DASH Segment Playback thread", myFuncName));
            playbackState = B_PlaybackMpegDashPlaybackState_eExit;
        }
#endif /* ==================== GARYWASHERE - End of Modified Code   ==================== */

        /* Entering trick mode (seek request). */
        if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode) {

            PRINTMSG_PLAYBACK(("%s: App is trying for a trickmode command, pause this thread", myFuncName));
            /* signal the control thread that we have paused and thus it can continue w/ setting up the trickmode operation */
            BKNI_ResetEvent(mpegDashSession->playbackThreadPauseDoneEvent);
            BKNI_SetEvent(mpegDashSession->playbackThreadPausedEvent);

            playbackState = B_PlaybackMpegDashPlaybackState_eTrickModeSuspend;
        }

        /*------------------------------------------------------------
         *  State: eInit (transient): This state only occurs once... at
         *  program startup.,
         *------------------------------------------------------------*/
        if (playbackState == B_PlaybackMpegDashPlaybackState_eInit) {
            PRINTMSG_PLAYBACK(("%s:%d : Handling Init state...", myFuncName, __LINE__ ));
            playbackState = B_PlaybackMpegDashPlaybackState_eStart;
        }

        /*------------------------------------------------------------
         *  State: eStart (transient): Start the process of feeding a
         *  downloaded Segment to the Playpump.
         *------------------------------------------------------------*/
        if (playbackState == B_PlaybackMpegDashPlaybackState_eStart) {
            PRINTMSG_PLAYBACK(("%s:%d : Handling Start state...", myFuncName, __LINE__ ));
            playbackState = B_PlaybackMpegDashPlaybackState_eSegBufWait;
        }

        /*------------------------------------------------------------
         *  State: eSegBufWait (stable): Wait in this state until we can
         *  find a SegmentBuffer that's filled with a downloaded
         *  segment.
         *------------------------------------------------------------*/
        if (playbackState == B_PlaybackMpegDashPlaybackState_eSegBufWait) {
            PRINTMSG_PLAYBACK(("%s:%d : Handling SegBufWait state...", myFuncName, __LINE__ ));

            /* I don't think we should ever have a SegmentBuffer here, but
             * if we do, just free it up and get a new one.  */
            if (segmentBuffer) {
                B_PlaybackIp_MpegDashSegmentBufferFree(playback_ip, segmentBuffer); segmentBuffer = NULL;
                if (mpegDashSession->downloadThreadWakeEvent)
                {
                    PRINTMSG_DOWNLOAD((":%d %s(): Setting downloadThreadWakeEvent.", __LINE__, myFuncName));
                    BKNI_SetEvent(mpegDashSession->downloadThreadWakeEvent);
                }
            }

            for (i=0; !segmentBuffer && i<MPEG_DASH_NUM_SEGMENT_BUFFERS; i++) {
                BKNI_AcquireMutex(mpegDashSession->segmentBuffer[i].lock);
                if (mpegDashSession->segmentBuffer[i].allocated && mpegDashSession->segmentBuffer[i].filled ) {
                    segmentBuffer = &mpegDashSession->segmentBuffer[i];
                    PRINTMSG_PLAYBACK(("%s: Buffer %p is filled, i:%d", myFuncName, (void *)segmentBuffer, i));
                }
                else {
                    PRINTMSG_PLAYBACK(("%s: Buffer %p is not filled, i:%d", myFuncName, (void *)&mpegDashSession->segmentBuffer[i], i));
                }
                BKNI_ReleaseMutex(mpegDashSession->segmentBuffer[i].lock);
            }
            BDBG_NUL(("%s:%d : Looking for segment buffer, got %p ", myFuncName, __LINE__ , (void *)segmentBuffer  ));

            if (segmentBuffer) {
                playbackState = B_PlaybackMpegDashPlaybackState_ePlaypumpBegin;
            }
            else {
                if (mpegDashSession->downloadedAllSegmentsInCurrentRepresentation) {
                    if (!mpegDashSession->mpdInfo->isDynamic && B_PlaybackIp_MpegDashEndOfStream(playback_ip) == true) {
                        /* MPD is "static" and we're done playing, so we can
                         * exit the playback thread.*/
                        PRINTMSG_PLAYBACK(("%s: Finished playing static MPD, so stopping the Segment Playback thread", myFuncName));
                        playbackState = B_PlaybackMpegDashPlaybackState_eExit;
                    }
                }

                if (mpegDashSession->mpegDashSegmentDownloadThreadDone) {
                    PRINTMSG_PLAYBACK(("%s: Network Read Error, wait to play out the stream", myFuncName));
                    if (B_PlaybackIp_MpegDashEndOfStream(playback_ip) == true) {
                        BDBG_WRN(("%s: No more segments to feed to playback & playback is done, total fed %lld", myFuncName, (long long)totalBytesFed));
                        playbackState = B_PlaybackMpegDashPlaybackState_eExit;
                    }
                    else {
                        BDBG_WRN(("%s: Continue waiting to either playout the whole stream or re-reading from socket incase it becomes valid again by a seek or rewind trickplay", myFuncName));
                    }
                }
            }
        }

        /*------------------------------------------------------------
         *  State: ePlaypumpBegin (transient): Do initialization for
         *  feeding to the Playpump.
         *------------------------------------------------------------*/
        if (playbackState == B_PlaybackMpegDashPlaybackState_ePlaypumpBegin) {
            PRINTMSG_PLAYBACK(("%s:%d : Handling PlaypumpBegin state...", myFuncName, __LINE__ ));

            segmentBytesFed = 0;
            PRINTMSG_PLAYBACK(("%s: Found a segment buffer: %p with %d bytes of%s%s", myFuncName, (void *)segmentBuffer, (int)segmentBuffer->bufferDepth,
                               segmentBuffer->hasAudio?" Audio":"", segmentBuffer->hasVideo?" Video":""));
            playbackState = B_PlaybackMpegDashPlaybackState_ePlaypumpFeed;
        }

        /*------------------------------------------------------------
         *  State: ePlaypumpFeed (transient): Try to feed a chunk of the
         *  data to the playpump.
         *------------------------------------------------------------*/
        if (playbackState == B_PlaybackMpegDashPlaybackState_ePlaypumpFeed) {
            NEXUS_PidType           pidType = segmentBuffer->hasAudio ? NEXUS_PidType_eAudio : NEXUS_PidType_eVideo;
            NEXUS_PlaypumpHandle    myPlaypump = B_PlaybackIp_MpegDashGetPlaypumpHandle(playback_ip,pidType);
            void                  * playpumpBuffer;
            ssize_t                 playpumpBufferSize;

            PRINTMSG_PLAYBACK(("%s:%d : Handling PlaypumpFeed state...", myFuncName, __LINE__ ));

            /* We'll go through a loop feeding "chunks" into the playpump
             * until either the playpump fills up, or until we run out of
             * data to feed to it. */

            for (;;) {
                ssize_t     bytesToCopy;
                ssize_t     bytesRemaining = segmentBuffer->bufferDepth - segmentBytesFed;

                if (NEXUS_Playpump_GetBuffer(myPlaypump, &playpumpBuffer, ((size_t *)&playpumpBufferSize))) {
                    BDBG_ERR(("%s:%d : Returned error from NEXUS_Playpump_GetBuffer()!", myFuncName, __LINE__ ));
                    goto error;
                }

                if (playpumpBufferSize == 0) {
                    /* If no space, just stay in this state and try again later. */
                    PRINTMSG_PLAYBACK(("%s: No space available in playpump... trying later.", myFuncName));
                    break;
                }

                /* Determine how much data can be copied. First choose optimal
                 * chunk size.*/
                bytesToCopy = (segmentBuffer->hasAudio) ? maxAudioFeedSize : maxVideoFeedSize;

                /* Reduce to playpump buffer size if necessary. */
                bytesToCopy = (playpumpBufferSize < bytesToCopy) ? playpumpBufferSize : bytesToCopy;

                /* Reduce to amount of data left to feed if necessary. */
                bytesToCopy = (bytesRemaining < bytesToCopy) ? bytesRemaining : bytesToCopy;


                memcpy(playpumpBuffer, segmentBuffer->buffer + segmentBytesFed, bytesToCopy);

                PRINTMSG_PLAYBACK(("%s: copied %d bytes from segment buffer into playpump buffer", myFuncName, (int)bytesToCopy));

                /* write data to file */
                if (playback_ip->enableRecording && fclear) {
                    fwrite(playback_ip->buffer, 1, bytesToCopy, fclear);
                }

                PRINTMSG_PLAYBACK(("%s:%d : Calling Playpump_ReadComplete for playpump %p hasAudio:%d hasVideo:%d",
                                   myFuncName, __LINE__,
                                   (void *)myPlaypump, segmentBuffer->hasAudio, segmentBuffer->hasVideo ));

                /* now feed appropriate data it to the playpump */
                if (NEXUS_Playpump_ReadComplete(myPlaypump, 0, bytesToCopy)) {
                    BDBG_ERR(("%s: NEXUS_Playpump_ReadComplete failed. Exiting.", myFuncName));
                    playbackState = B_PlaybackMpegDashPlaybackState_eExit;
                    BKNI_SetEvent(mpegDashSession->playbackThreadWakeEvent); /* Self-wake to process new state. */
                    continue;
                }

                segmentBytesFed += bytesToCopy;
                totalBytesFed += bytesToCopy;

                PRINTMSG_PLAYBACK(("%s:%d : Playpump Buffer sent, bytes sent %d out of %d, %d remaining", myFuncName, __LINE__, (int)bytesToCopy, (int)segmentBuffer->bufferDepth, (int)(segmentBuffer->bufferDepth - segmentBytesFed )));

                if (segmentBytesFed >= segmentBuffer->bufferDepth) {
                    PRINTMSG_PLAYBACK(("%s: Finished feeding %s Segment (%d bytes) to Playpump", myFuncName, segmentBuffer->hasAudio?"Audio":"Video", (int)segmentBytesFed));
                    playbackState = B_PlaybackMpegDashPlaybackState_eSegBufFree;    /* We're done here, on to next state. */
                    break;
                }
            }/* End of segment-feeding loop. */
        }

        /*------------------------------------------------------------
         *  State: eSegBufFree (transient): The SegmentBuffer has been
         *  completely fed to the playpump, so we can free it up (and
         *  let the Download thread use it).
         *------------------------------------------------------------*/
        if (playbackState == B_PlaybackMpegDashPlaybackState_eSegBufFree) {
            PRINTMSG_PLAYBACK(("%s:%d : Handling SegBufFree state...", myFuncName, __LINE__ ));

            PRINTMSG_PLAYBACK(("%s:%d : Freeing Segment Buffer, setting bufferEmptiedEvent", myFuncName, __LINE__));
            /* Inform MPEG-DASH Download thread that buffer is emptied and
             * fed to the playback h/w*/
            B_PlaybackIp_MpegDashSegmentBufferFree(playback_ip, segmentBuffer); segmentBuffer = NULL;

            if (mpegDashSession->downloadThreadWakeEvent)
            {
                PRINTMSG_DOWNLOAD((":%d %s(): Setting downloadThreadWakeEvent.", __LINE__, myFuncName));
                BKNI_SetEvent(mpegDashSession->downloadThreadWakeEvent);
            }

            /* Set to the Start state and wake ourself up in case there's
             * another SegmentBuffer ready to be fed to the playpump. */
            playbackState = B_PlaybackMpegDashPlaybackState_eStart;
            PRINTMSG_PLAYBACK(("%s:%d: Setting playbackThreadWakeEvent to check for another filled SegmentBuffer.", myFuncName, __LINE__));
            BKNI_SetEvent(mpegDashSession->playbackThreadWakeEvent); /* Self-wake to process new state. */
        }

        /*------------------------------------------------------------
         *  State: eTrickModeSuspend (stable):
         *------------------------------------------------------------*/
        if (playbackState == B_PlaybackMpegDashPlaybackState_eTrickModeSuspend)
        {
            PRINTMSG_PLAYBACK(("%s:%d : Handling TrickModeSuspend state...", myFuncName, __LINE__ ));

            /* wait on signal from the control thread to indicate trickmode work is done */
            rc = BKNI_WaitForEvent(mpegDashSession->playbackThreadPauseDoneEvent, 100 * MPEG_DASH_EVENT_TIMEOUT_MSEC);
            BKNI_ResetEvent(mpegDashSession->playbackThreadPausedEvent);  /* Not in paused state anymore. */
            if (rc == BERR_TIMEOUT || rc!=0) {
                BDBG_ERR(("%s: EVENT %s: failed to receive event from control thread indicating trickmode completion", myFuncName, rc==BERR_TIMEOUT?"Timeout":"Error"));
                goto error;
            }
            PRINTMSG_PLAYBACK(("%s:%d : Resuming from TrickModeSuspend ", myFuncName, __LINE__   ));
            playbackState = B_PlaybackMpegDashPlaybackState_eReset;
        }

        /*------------------------------------------------------------
         *  State: eReset (transient):
         *  State: eExit  (transient):
         *------------------------------------------------------------*/
        if (playbackState == B_PlaybackMpegDashPlaybackState_eReset ||
            playbackState == B_PlaybackMpegDashPlaybackState_eExit)     {
            PRINTMSG_PLAYBACK(("%s:%d : Handling Reset/Exit state...", myFuncName, __LINE__ ));

            if (segmentBuffer) {
                B_PlaybackIp_MpegDashSegmentBufferFree(playback_ip, segmentBuffer); segmentBuffer = NULL;
            }

            /* We're done cleaning things up.  If this is the eExit state,
             * then exit the state processing loop.  */
            if (playbackState == B_PlaybackMpegDashPlaybackState_eExit ) {
                break;
            }

            /* Otherwise, this is the eReset state so just take it from
             * the top...*/
            playbackState = B_PlaybackMpegDashPlaybackState_eStart;
            PRINTMSG_PLAYBACK(("%s:%d: Setting playbackThreadWakeEvent after eReset state.", myFuncName, __LINE__));
            BKNI_SetEvent(mpegDashSession->playbackThreadWakeEvent); /* Self-wake to process new state. */
        }
    }

    PRINTMSG_PLAYBACK(("%s: Normal exit from state processing loop...", myFuncName));



error:
    mpegDashSession->mpegDashPlaybackThreadDone = true;         /* This will cause download thread to exit. */
        PRINTMSG_DOWNLOAD((":%d %s(): Setting downloadThreadWakeEvent.", __LINE__, myFuncName));
    BKNI_SetEvent(mpegDashSession->downloadThreadWakeEvent);    /* Wake up the download thread. */

    while (!mpegDashSession->mpegDashSegmentDownloadThreadDone) {
        int count = 0;
        if (count++ > 50) {
            BDBG_ERR(("%s: Failed to wait for MPEG-DASH Media Segment Download thread to finish for over %d attempts", myFuncName, count));
            break;
        }
        PRINTMSG_PLAYBACK(("%s: MPEG-DASH Playback thread ran into some error, wait for Download thread to finish", myFuncName));
        BKNI_Sleep(100);
    }
    if (mpegDashSession->mpegDashMpdReDownloadThread) {
        while (!mpegDashSession->mpegDashMpdReDownloadThreadDone) {
            int count = 0;
            BKNI_SetEvent(mpegDashSession->reDownloadMpdEvent);
            if (count++ > 50) {
                BDBG_ERR(("%s: Failed to wait for MPEG-DASH MPD ReDownload thread to finish for over %d attempts", myFuncName, count));
                break;
            }
            PRINTMSG_PLAYBACK(("%s: MPEG-DASH Playback thread ran into some error, wait for MPD ReDownload thread to finish", myFuncName));
            BKNI_Sleep(100);
        }
    }
    if (playback_ip->enableRecording && fclear) {
        fflush(fclear);
        fclose(fclear);
    }
    if (mpegDashSession->mpegDashSegmentDownloadThread) {
        B_Thread_Destroy(mpegDashSession->mpegDashSegmentDownloadThread);
    }

    if (mpegDashSession->mpegDashMpdReDownloadThread) {
        B_Thread_Destroy(mpegDashSession->mpegDashMpdReDownloadThread);
    }

    if (playback_ip->openSettings.eventCallback &&
            playback_ip->playback_state != B_PlaybackIpState_eStopping &&
            playback_ip->playback_state != B_PlaybackIpState_eStopped)
    {
        B_PlaybackIpEventIds eventId;
        if (playback_ip->serverClosed)
            eventId = B_PlaybackIpEvent_eServerEndofStreamReached;
        else
            eventId = B_PlaybackIpEvent_eErrorDuringStreamPlayback;
            eventId = B_PlaybackIpEvent_eServerEndofStreamReached;
        playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, eventId);
    }
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: MPEG-DASH Playback thread is exiting...", myFuncName));
#endif
    BKNI_SetEvent(playback_ip->playback_halt_event);

    BDBG_LEAVE(B_PlaybackIp_MpegDashPlaybackThread);
}
