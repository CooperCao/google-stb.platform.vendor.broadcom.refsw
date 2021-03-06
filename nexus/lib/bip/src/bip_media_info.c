/******************************************************************************
 * (c) 2016 Broadcom Corporation
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
#include "bip_priv.h"
#include "bip_media_info.h"

#include "bfile_stdio.h"
#include "bmpeg2ts_probe.h"
#include "bmpeg2ts_psi_probe.h"
#include "bmpeg_video_probe.h"
#include "bhevc_video_probe.h"
#include "b_playback_ip_lib.h"

#include "b_psip_lib.h"
#ifdef STREAMER_CABLECARD_SUPPORT
#include "ip_strm_cablecard.h"
#else
#include <tspsimgr2.h>
#endif

#include "bip_xml.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

BDBG_MODULE( bip_media_info );
BDBG_OBJECT_ID( BIP_MediaInfo );

BIP_SETTINGS_ID(BIP_MediaInfoCreateSettings);
BIP_SETTINGS_ID(BIP_MediaInfoMakeNavForTsSettings);

/* XML tags */
#define BIP_MEDIAINFO_XML_TAG_MEDIA_INFO             "BIP_MediaInfo" /* root element */
#define BIP_MEDIAINFO_XML_TAG_SIZE_IN_BYTES          "sizeInBytes"   /* File size in bytes */
#define BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TYPE        "mediaInfoType" /* whether stream or unknown. */


#define BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_STREAM      "BIP_MediaInfoStream"
#define BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_GROUP   "BIP_MediaInfoTrackGroup"
#define BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_GROUP_TS   "BIP_MediaInfoTrackGroup_Ts"

#define BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK         "BIP_MediaInfoTrack"
#define BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_TS      "Ts"
#define BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_VIDEO   "BIP_MediaInfoVideoTrack"
#define BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_AUDIO   "BIP_MediaInfoAudioTrack"


/* XML attributes */
#define BIP_MEDIAINFO_XML_ATT_VERSION           "version"
#define BIP_MEDIAINFO_XML_ATT_ABS_MEDIA_PATH    "pMediaFileAbsolutePathname"

#define BIP_MEDIAINFO_STREAM_XML_ATT_TYPE                   "transportType"
#define BIP_MEDIAINFO_STREAM_XML_ATT_NUM_GROUPS             "numberOfTrackGroups"
#define BIP_MEDIAINFO_STREAM_XML_ATT_NUM_TRKS               "numberOfTracks"
#define BIP_MEDIAINFO_STREAM_XML_ATT_AVG_BITRATE            "avgBitRate"
#define BIP_MEDIAINFO_STREAM_XML_ATT_MAX_BITRATE            "maxBitRate"
#define BIP_MEDIAINFO_STREAM_XML_ATT_DURATION               "durationInMs"
#define BIP_MEDIAINFO_STREAM_XML_ATT_CONTENTLENGTH          "contentLength"
#define BIP_MEDIAINFO_STREAM_XML_ATT_LIVECHANNEL            "liveChannel"
#define BIP_MEDIAINFO_STREAM_XML_ATT_TTS_ENABLED            "transportTimeStampEnabled"

#define BIP_MEDIAINFO_TRK_GROUP_XML_ATT_ID                "trackGroupId"
#define BIP_MEDIAINFO_TRK_GROUP_XML_ATT_NUM_TRKS          "numberOfTracks"
#define BIP_MEDIAINFO_TRK_GROUP_XML_ATT_PMT_PID           "pmtPid"

#define BIP_MEDIAINFO_TRK_XML_ATT_TYPE                    "trackType"
#define BIP_MEDIAINFO_TRK_XML_ATT_ID                      "trackId"
#define BIP_MEDIAINFO_TRK_XML_ATT_PARSED_PAYLOAD          "parsedPayload"

#define BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_CODEC             "codec"
#define BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_WIDTH             "width"
#define BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_HEIGHT            "height"
#define BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_BITRATE           "bitrate"
#define BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_FRAMERATE         "framerate"
#define BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_COLORDEPTH        "colorDepth"

#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_CODEC             "codec"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_CHANNELCOUNT      "channelCount"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_SAMPLESIZE        "sampleSize"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_BITRATE           "bitrate"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_SAMPLERATE        "sampleRate"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_LANGUAGE          "language"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_SESSION_ENABLED "hlsSessionEnabled"

#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MPEGTS            "mpegTsAudio"       /* we consider this as only mpegTs and not mpegTs part of hls.*/
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MPEGTS_AUDIOTYPE  "mpegTsAudioType"   /* Mpeg2Ts container contains audipType with language . Please refer table 2.53, iso13818-1*/
                                                                              /* like : 0x00    undefined
                                                                                  0x01  clean effects
                                                                                  0x02  hearing impaired
                                                                                  0x03  visual impaired commentary
                                                                                  0x04-0xFF reserved */

#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS                         "hlsAudio"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_DEFAULTAUDIOFLAG        "hlsDefaultAudioFlag"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_PID                     "hlsAudioPid"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_LANGUAGE_CODE           "hlsLanguageCode"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_LANGUAGE_DESCRIPTION    "hlsLanguageDescription"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_REQUIRED_2ND_PLAYPUMP   "hlsAudioRequired2NdPlayPump"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_GROUP_ID                "hlsAudioGroupId"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_EXTRA_AUDIO_SPECIFIC_CONTAINER_TYPE "hlsExtraAudioSpecificContainerType"

#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_AC3             "ac3"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_AC3_BSMOD       "ac3Bsmod"


#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MAINAUDIO       "mainAudio"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MAINAUDIOID     "mainAudioId"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_ASVCFLAGS       "asvcflags"
#define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_LANGUAGE2       "language2"



#define BIP_MEDIAINFO_TRACKLIST_INSERT(listHeadAddress, listEntry)          \
do                                                                          \
{                                                                           \
    BIP_MediaInfoTrack **ppCurrentNode = NULL;                              \
    ppCurrentNode = (listHeadAddress);                                      \
    while(*ppCurrentNode != NULL)                                           \
    {                                                                       \
        ppCurrentNode = &((*ppCurrentNode)->pNextTrackForStream);           \
    }                                                                       \
    *ppCurrentNode = listEntry;                                             \
}while (0)

#define BIP_MEDIAINFO_TRACKGROUP_INSERT_TRACK(listHeadAddress, listEntry)   \
do                                                                          \
{                                                                           \
    BIP_MediaInfoTrack **ppCurrentNode = NULL;                              \
    ppCurrentNode = (listHeadAddress);                                      \
    while(*ppCurrentNode != NULL)                                           \
    {                                                                       \
        ppCurrentNode = &((*ppCurrentNode)->pNextTrackForTrackGroup);       \
    }                                                                       \
    *ppCurrentNode = listEntry;                                             \
}while (0)

#define BIP_MEDIAINFO_GROUPLIST_INSERT(listHeadAddress, listEntry)  \
do                                                                  \
{                                                                   \
    BIP_MediaInfoTrackGroup **ppCurrentNode = NULL;                 \
    ppCurrentNode = (listHeadAddress);                              \
    while(*ppCurrentNode != NULL)                                   \
    {                                                               \
        ppCurrentNode = &((*ppCurrentNode)->pNextTrackGroup);       \
    }                                                               \
    *ppCurrentNode = listEntry;                                     \
}while (0)

/* First search whether a trackGroup exist for the specified groupId.
   If existing track group not found,Create a new track group for the specified trackGroup id
   and add it to the trackGroup list.*/
#define BIP_MEDIAINFO_GET_TRACKGROUP_FOR_GROUPID(listHeadAddress, listNode, groupId, pointerToGroupCounter)  \
do                                                                                  \
{                                                                                   \
    BIP_MediaInfoTrackGroup   **ppTrackGroup = NULL;                                \
    unsigned *pGrpCounter = NULL;                                                   \
    pGrpCounter = (pointerToGroupCounter);                                          \
    ppTrackGroup = (listHeadAddress);                                               \
    while(*ppTrackGroup)                                                            \
    {                                                                               \
        if((*ppTrackGroup)->trackGroupId == groupId)                                \
        {                                                                           \
            break;                                                                  \
        }                                                                           \
        ppTrackGroup = &((*ppTrackGroup)->pNextTrackGroup);                         \
    }                                                                               \
    if(*ppTrackGroup == NULL)                                                       \
    {                                                                               \
        listNode = B_Os_Calloc(1, sizeof(BIP_MediaInfoTrackGroup));                 \
        BIP_CHECK_GOTO(( pTrackGroup!=NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfoTrackGroup", sizeof(BIP_MediaInfoTrackGroup)  ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );  \
        listNode->trackGroupId = groupId;                                           \
        *ppTrackGroup = listNode;                                                   \
        (*pGrpCounter)++;                                                           \
        BDBG_MSG((BIP_MSG_PRE_FMT "pTrackGroup------------------->%p: pGrpCounter = %d" BIP_MSG_PRE_ARG, listNode, *pGrpCounter));                   \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        listNode = *ppTrackGroup;                                                   \
    }                                                                               \
}while (0)

static struct {
    NEXUS_VideoCodec nexus;

    bvideo_codec settop;
} g_videoCodec[] = {
    {NEXUS_VideoCodec_eUnknown,       bvideo_codec_none          },
    {NEXUS_VideoCodec_eUnknown,       bvideo_codec_unknown       },
    {NEXUS_VideoCodec_eMpeg1,         bvideo_codec_mpeg1         },
    {NEXUS_VideoCodec_eMpeg2,         bvideo_codec_mpeg2         },
    {NEXUS_VideoCodec_eMpeg4Part2,    bvideo_codec_mpeg4_part2   },
    {NEXUS_VideoCodec_eH263,          bvideo_codec_h263          },
    {NEXUS_VideoCodec_eH264,          bvideo_codec_h264          },
    {NEXUS_VideoCodec_eH265,          bvideo_codec_h265          },
    {NEXUS_VideoCodec_eVc1,           bvideo_codec_vc1           },
    {NEXUS_VideoCodec_eVc1SimpleMain, bvideo_codec_vc1_sm        },
    {NEXUS_VideoCodec_eDivx311,       bvideo_codec_divx_311      },
    {NEXUS_VideoCodec_eH264_Svc,      bvideo_codec_h264_svc      },
    {NEXUS_VideoCodec_eH264_Mvc,      bvideo_codec_h264_mvc      },
    {NEXUS_VideoCodec_eAvs,           bvideo_codec_avs           },
    {NEXUS_VideoCodec_eSpark,         bvideo_codec_spark         },
    {NEXUS_VideoCodec_eVp6,           bvideo_codec_vp6           },
    {NEXUS_VideoCodec_eRv40,          bvideo_codec_rv40          },
    {NEXUS_VideoCodec_eVp8,           bvideo_codec_vp8           },
    {NEXUS_VideoCodec_eVp9,           bvideo_codec_vp9           },
    {NEXUS_VideoCodec_eSpark,         bvideo_codec_spark         },
    {NEXUS_VideoCodec_eMotionJpeg,    bvideo_codec_mjpeg         }
};

static struct {
    NEXUS_AudioCodec nexus;
    baudio_format    settop;
} g_audioCodec[] = {
    {NEXUS_AudioCodec_eUnknown,     baudio_format_unknown      },
    {NEXUS_AudioCodec_eMpeg,        baudio_format_mpeg         },
    {NEXUS_AudioCodec_eMp3,         baudio_format_mp3          },
    {NEXUS_AudioCodec_eAac,         baudio_format_aac          },
    {NEXUS_AudioCodec_eAacPlus,     baudio_format_aac_plus     },
    {NEXUS_AudioCodec_eAacPlusAdts, baudio_format_aac_plus_adts},
    {NEXUS_AudioCodec_eAacPlusLoas, baudio_format_aac_plus_loas},
    {NEXUS_AudioCodec_eAc3,         baudio_format_ac3          },
    {NEXUS_AudioCodec_eAc3Plus,     baudio_format_ac3_plus     },
    {NEXUS_AudioCodec_eDts,         baudio_format_dts          },
    {NEXUS_AudioCodec_eLpcmHdDvd,   baudio_format_lpcm_hddvd   },
    {NEXUS_AudioCodec_eLpcmBluRay,  baudio_format_lpcm_bluray  },
    {NEXUS_AudioCodec_eDtsHd,       baudio_format_dts_hd       },
    {NEXUS_AudioCodec_eWmaStd,      baudio_format_wma_std      },
    {NEXUS_AudioCodec_eWmaPro,      baudio_format_wma_pro      },
    {NEXUS_AudioCodec_eLpcmDvd,     baudio_format_lpcm_dvd     },
    {NEXUS_AudioCodec_eAvs,         baudio_format_avs          },
    {NEXUS_AudioCodec_ePcmWav,      baudio_format_pcm          },
    {NEXUS_AudioCodec_eAmr,         baudio_format_amr          },
    {NEXUS_AudioCodec_eDra,         baudio_format_dra          },
    {NEXUS_AudioCodec_eCook,        baudio_format_cook         },
    {NEXUS_AudioCodec_eVorbis,      baudio_format_vorbis       },
    {NEXUS_AudioCodec_eLpcm1394,    baudio_format_lpcm_1394    },
    {NEXUS_AudioCodec_eFlac,        baudio_format_flac         },
    {NEXUS_AudioCodec_eApe,         baudio_format_ape          },
    {NEXUS_AudioCodec_eMlp,         baudio_format_mlp          },
    {NEXUS_AudioCodec_eG711,        baudio_format_g711         },
};

static struct {
    NEXUS_TransportType nexus;
    bstream_mpeg_type   settop;
} g_mpegType[] = {
    {NEXUS_TransportType_eUnknown,  bstream_mpeg_type_unknown      },
    {NEXUS_TransportType_eEs,       bstream_mpeg_type_es           },
    {NEXUS_TransportType_eTs,       bstream_mpeg_type_bes          },
    {NEXUS_TransportType_eMpeg2Pes, bstream_mpeg_type_pes          },
    {NEXUS_TransportType_eTs,       bstream_mpeg_type_ts           },
    {NEXUS_TransportType_eDssEs,    bstream_mpeg_type_dss_es       },
    {NEXUS_TransportType_eDssPes,   bstream_mpeg_type_dss_pes      },
    {NEXUS_TransportType_eVob,      bstream_mpeg_type_vob          },
    {NEXUS_TransportType_eAsf,      bstream_mpeg_type_asf          },
    {NEXUS_TransportType_eAvi,      bstream_mpeg_type_avi          },
    {NEXUS_TransportType_eMpeg1Ps,  bstream_mpeg_type_mpeg1        },
    {NEXUS_TransportType_eMp4,      bstream_mpeg_type_mp4          },
    {NEXUS_TransportType_eMkv,      bstream_mpeg_type_mkv          },
    {NEXUS_TransportType_eWav,      bstream_mpeg_type_wav          },
    {NEXUS_TransportType_eFlv,      bstream_mpeg_type_flv          },
    {NEXUS_TransportType_eRmff,     bstream_mpeg_type_rmff         },
    {NEXUS_TransportType_eOgg,      bstream_mpeg_type_ogg          },
    {NEXUS_TransportType_eFlac,     bstream_mpeg_type_flac         },
    {NEXUS_TransportType_eAmr,      bstream_mpeg_type_amr          },
    {NEXUS_TransportType_eApe,      bstream_mpeg_type_ape          }
};

/* Defined as per the specification of ISO 6391-2 code and ISO 639-1 code*/
typedef struct BIP_MediaInfoLanguageCode
{
    const char *pCode6392;
    const char *pCode6391;
}BIP_MediaInfoLanguageCode;

static const BIP_MediaInfoLanguageCode mediaInfoLanguageCode[] = {
{ "aar" ,"aa" },
{ "abk" ,"ab" },
{ "afr" ,"af" },
{ "aka" ,"ak" },
{ "sqi" ,"sq" },
{ "amh" ,"am" },
{ "ara" ,"ar" },
{ "arg" ,"an" },
{ "hye" ,"hy" },
{ "asm" ,"as" },
{ "ava" ,"av" },
{ "ave" ,"ae" },
{ "aym" ,"ay" },
{ "aze" ,"az" },
{ "bak" ,"ba" },
{ "bam" ,"bm" },
{ "eus" ,"eu" },
{ "bel" ,"be" },
{ "ben" ,"bn" },
{ "bih" ,"bh" },
{ "bis" ,"bi" },
{ "bod" ,"bo" },
{ "bos" ,"bs" },
{ "bre" ,"br" },
{ "bul" ,"bg" },
{ "mya" ,"my" },
{ "cat" ,"ca" },
{ "ces" ,"cs" },
{ "cha" ,"ch" },
{ "che" ,"ce" },
{ "zho" ,"zh" },
{ "chu" ,"cu" },
{ "chv" ,"cv" },
{ "cor" ,"kw" },
{ "cos" ,"co" },
{ "cre" ,"cr" },
{ "cym" ,"cy" },
{ "ces" ,"cs" },
{ "dan" ,"da" },
{ "deu" ,"de" },
{ "div" ,"dv" },
{ "nld" ,"nl" },
{ "dzo" ,"dz" },
{ "ell" ,"el" },
{ "eng" ,"en" },
{ "epo" ,"eo" },
{ "est" ,"et" },
{ "eus" ,"eu" },
{ "ewe" ,"ee" },
{ "fao" ,"fo" },
{ "fas" ,"fa" },
{ "fij" ,"fj" },
{ "fin" ,"fi" },
{ "fra" ,"fr" },
{ "fra" ,"fr" },
{ "fry" ,"fy" },
{ "ful" ,"ff" },
{ "kat" ,"ka" },
{ "deu" ,"de" },
{ "gla" ,"gd" },
{ "gle" ,"ga" },
{ "glg" ,"gl" },
{ "glv" ,"gv" },
{ "ell" ,"el" },
{ "grn" ,"gn" },
{ "guj" ,"gu" },
{ "hat" ,"ht" },
{ "hau" ,"ha" },
{ "heb" ,"he" },
{ "her" ,"hz" },
{ "hin" ,"hi" },
{ "hmo" ,"ho" },
{ "hrv" ,"hr" },
{ "hun" ,"hu" },
{ "hye" ,"hy" },
{ "ibo" ,"ig" },
{ "isl" ,"is" },
{ "ido" ,"io" },
{ "iii" ,"ii" },
{ "iku" ,"iu" },
{ "ile" ,"ie" },
{ "ina" ,"ia" },
{ "ind" ,"id" },
{ "ipk" ,"ik" },
{ "isl" ,"is" },
{ "ita" ,"it" },
{ "jav" ,"jv" },
{ "jpn" ,"ja" },
{ "kal" ,"kl" },
{ "kan" ,"kn" },
{ "kas" ,"ks" },
{ "kat" ,"ka" },
{ "kau" ,"kr" },
{ "kaz" ,"kk" },
{ "khm" ,"km" },
{ "kik" ,"ki" },
{ "kin" ,"rw" },
{ "kir" ,"ky" },
{ "kom" ,"kv" },
{ "kon" ,"kg" },
{ "kor" ,"ko" },
{ "kua" ,"kj" },
{ "kur" ,"ku" },
{ "lao" ,"lo" },
{ "lat" ,"la" },
{ "lav" ,"lv" },
{ "lim" ,"li" },
{ "lin" ,"ln" },
{ "lit" ,"lt" },
{ "ltz" ,"lb" },
{ "lub" ,"lu" },
{ "lug" ,"lg" },
{ "mkd" ,"mk" },
{ "mah" ,"mh" },
{ "mal" ,"ml" },
{ "mri" ,"mi" },
{ "mar" ,"mr" },
{ "msa" ,"ms" },
{ "mkd" ,"mk" },
{ "mlg" ,"mg" },
{ "mlt" ,"mt" },
{ "mon" ,"mn" },
{ "mri" ,"mi" },
{ "msa" ,"ms" },
{ "mya" ,"my" },
{ "nau" ,"na" },
{ "nav" ,"nv" },
{ "nbl" ,"nr" },
{ "nde" ,"nd" },
{ "ndo" ,"ng" },
{ "nep" ,"ne" },
{ "nld" ,"nl" },
{ "nno" ,"nn" },
{ "nob" ,"nb" },
{ "nor" ,"no" },
{ "nya" ,"ny" },
{ "oci" ,"oc" },
{ "oji" ,"oj" },
{ "ori" ,"or" },
{ "orm" ,"om" },
{ "oss" ,"os" },
{ "pan" ,"pa" },
{ "fas" ,"fa" },
{ "pli" ,"pi" },
{ "pol" ,"pl" },
{ "por" ,"pt" },
{ "pus" ,"ps" },
{ "que" ,"qu" },
{ "roh" ,"rm" },
{ "ron" ,"ro" },
{ "run" ,"rn" },
{ "rus" ,"ru" },
{ "sag" ,"sg" },
{ "san" ,"sa" },
{ "sin" ,"si" },
{ "slk" ,"sk" },
{ "slk" ,"sk" },
{ "slv" ,"sl" },
{ "sme" ,"se" },
{ "smo" ,"sm" },
{ "sna" ,"sn" },
{ "snd" ,"sd" },
{ "som" ,"so" },
{ "sot" ,"st" },
{ "spa" ,"es" },
{ "sqi" ,"sq" },
{ "srd" ,"sc" },
{ "srp" ,"sr" },
{ "ssw" ,"ss" },
{ "sun" ,"su" },
{ "swa" ,"sw" },
{ "swe" ,"sv" },
{ "tah" ,"ty" },
{ "tam" ,"ta" },
{ "tat" ,"tt" },
{ "tel" ,"te" },
{ "tgk" ,"tg" },
{ "tgl" ,"tl" },
{ "tha" ,"th" },
{ "bod" ,"bo" },
{ "tir" ,"ti" },
{ "ton" ,"to" },
{ "tsn" ,"tn" },
{ "tso" ,"ts" },
{ "tuk" ,"tk" },
{ "tur" ,"tr" },
{ "twi" ,"tw" },
{ "uig" ,"ug" },
{ "ukr" ,"uk" },
{ "urd" ,"ur" },
{ "uzb" ,"uz" },
{ "ven" ,"ve" },
{ "vie" ,"vi" },
{ "vol" ,"vo" },
{ "cym" ,"cy" },
{ "wln" ,"wa" },
{ "wol" ,"wo" },
{ "xho" ,"xh" },
{ "yid" ,"yi" },
{ "yor" ,"yo" },
{ "zha" ,"za" },
{ "zho" ,"zh" },
{ "zul" ,"zu" }
}; /* BIP_MediaInfoLanguageCode*/

const char * BIP_MediaInfo_GetIso6392CodeFrom6391Code(const char *pCode)
{
    unsigned i;
    for(i=0; mediaInfoLanguageCode[i].pCode6391; i++)
    {
        if(!strcasecmp(mediaInfoLanguageCode[i].pCode6391, pCode))
        {
            return mediaInfoLanguageCode[i].pCode6392;
        }
    }
    return NULL;
}

/**********************************************
  * start BIP_MediaInfo_Convert functions
  *********************************************/
#define BIP_MEDIAINFO_CONVERT( g_struct )                                              \
    unsigned i;                                                                        \
    for (i = 0; i<sizeof( g_struct )/sizeof( g_struct[0] ); i++) {                     \
        if (g_struct[i].settop == settop_value) {                                      \
            return g_struct[i].nexus;                                                  \
        }                                                                              \
    }                                                                                  \
    BDBG_ERR(( "unable to find Settop API value %d in %s", settop_value, #g_struct )); \
    return g_struct[0].nexus

static NEXUS_VideoCodec BIP_MediaInfo_ConvertBmediaVideoCodecToNexus(
    bvideo_codec settop_value
    )
{
    BIP_MEDIAINFO_CONVERT( g_videoCodec );
}

static NEXUS_AudioCodec BIP_MediaInfo_ConvertBmediaAudioCodecToNexus(
    baudio_format settop_value
    )
{
    BIP_MEDIAINFO_CONVERT( g_audioCodec );
}

static NEXUS_TransportType BIP_MediaInfo_ConvertBmediaMpegTypeToNexus(
    bstream_mpeg_type settop_value
    )
{
    BIP_MEDIAINFO_CONVERT( g_mpegType );
}

static void populateMediaInfoTrack(
    BIP_MediaInfoTrack   *pMediaInfoTrack,
    const bmedia_probe_track  *pTrack,
    const bmedia_probe_stream *pStream
    )
{
    pMediaInfoTrack->trackId = pTrack->number;
    pMediaInfoTrack->trackType = pTrack->type;

    if(pTrack->type == bmedia_track_type_video)
    {
        pMediaInfoTrack->info.video.bitrate = pTrack->info.video.bitrate*1000;
        pMediaInfoTrack->info.video.codec = BIP_MediaInfo_ConvertBmediaVideoCodecToNexus(pTrack->info.video.codec);
        pMediaInfoTrack->info.video.height = pTrack->info.video.height;
        pMediaInfoTrack->info.video.width = pTrack->info.video.width;

        if (pMediaInfoTrack->info.video.codec == NEXUS_VideoCodec_eMpeg2)
        {
            pMediaInfoTrack->info.video.frameRate = ((bmedia_probe_mpeg_video *)&pTrack->info.video.codec_specific )->framerate;
#if 0
            if (pMediaInfoTrack->info.video.frameRate <= 0)
            {
                /*TODO: Check whetherv this is required. Need to also check why Randy has added this check. Was there any issue?*/
                pMediaInfoTrack->info.video.frameRate = 30000;
            }
#endif
        }
        if (pTrack->info.video.codec == bvideo_codec_h265)
        {
            pMediaInfoTrack->info.video.colorDepth = ((bmedia_probe_h265_video *)&pTrack->info.video.codec_specific )->sps.bit_depth_luma;
        }
    }
    else if(pTrack->type == bmedia_track_type_audio)
    {
        pMediaInfoTrack->info.audio.bitrate = pTrack->info.audio.bitrate*1000;
        pMediaInfoTrack->info.audio.channelCount = pTrack->info.audio.channel_count;
        pMediaInfoTrack->info.audio.codec = BIP_MediaInfo_ConvertBmediaAudioCodecToNexus(pTrack->info.audio.codec);
        pMediaInfoTrack->info.audio.sampleRate = pTrack->info.audio.sample_rate;
        pMediaInfoTrack->info.audio.sampleSize = pTrack->info.audio.sample_size;

        if(pStream->type == bstream_mpeg_type_ts)
        {
            if( (pStream->probe_id == BMPEG2TS_PSI_PROBE_ID || pStream->probe_id == BMPEG2TS192_PSI_PROBE_ID))
            {
                bmpeg2ts_psi_probe_track *pTrackMpeg2TS = (bmpeg2ts_psi_probe_track *)pTrack;
                pMediaInfoTrack->info.audio.pLanguage = B_Os_Calloc(1,BIP_MEDIA_INFO_LANGUAGE_FIELD_SIZE);
                memcpy( pMediaInfoTrack->info.audio.pLanguage, pTrackMpeg2TS->language.code, BIP_MEDIA_INFO_LANGUAGE_FIELD_SIZE);

                if(pTrackMpeg2TS->ac3_bsmod.valid)
                {
                    pMediaInfoTrack->info.audio.descriptor.ac3.bsmodValid = pTrackMpeg2TS->ac3_bsmod.valid;
                    pMediaInfoTrack->info.audio.descriptor.ac3.bsmod = pTrackMpeg2TS->ac3_bsmod.bsmod;
                }
            }
        }
    }

    if(pStream->type == bstream_mpeg_type_ts)
    {
        if( (pStream->probe_id == BMPEG2TS_PSI_PROBE_ID || pStream->probe_id == BMPEG2TS192_PSI_PROBE_ID))
        {
            bmpeg2ts_psi_probe_track *pTrackMpeg2TS = (bmpeg2ts_psi_probe_track *)pTrack;
            pMediaInfoTrack->parsedPayload = pTrackMpeg2TS->parsed_payload;
        }
        else
        {
            bmpeg2ts_probe_track *pTrackMpeg2TS = (bmpeg2ts_probe_track *)pTrack;
            pMediaInfoTrack->parsedPayload = pTrackMpeg2TS->parsed_payload;
        }
    }
}

/******************************************************************
  * Add media track to trackGroup since track has a valid GroupId.
  ****************************************************************/
static BIP_Status addMediaInfoTrackToTrackGroup(
    BIP_MediaInfoStream  *pMediaInfoStream,
    BIP_MediaInfoTrack   *pMediaInfoTrack,
    unsigned programNumber                            /* For every new programNumber a new trackGroup will be created.*/
    )
{
    BIP_Status       rc = BIP_SUCCESS;
    BIP_MediaInfoTrackGroup *pTrackGroup = NULL;

    BDBG_ASSERT( NULL != pMediaInfoStream );
    BDBG_ASSERT( NULL != pMediaInfoTrack );

    /* This macro internally use the BIP_Status variable and the error label. */
    BIP_MEDIAINFO_GET_TRACKGROUP_FOR_GROUPID(&pMediaInfoStream->pFirstTrackGroupInfo, pTrackGroup, programNumber, &pMediaInfoStream->numberOfTrackGroups);

    BIP_MEDIAINFO_TRACKGROUP_INSERT_TRACK(&pTrackGroup->pFirstTrackForTrackGroup, pMediaInfoTrack);
    pMediaInfoTrack->pParentTrackGroup = pTrackGroup;
    pTrackGroup->numberOfTracks++;
error:
    return rc;
}

/******************************************************************
  * Create media Info track.
  ****************************************************************/
static BIP_Status createMediaInfoTrack(
    BIP_MediaInfoStream  *pMediaInfoStream,
    const bmedia_probe_stream *pStream,
    const bmedia_probe_track   *pTrack
    )
{
   BIP_Status rc    = BIP_SUCCESS;
   BIP_MediaInfoTrack   *pMediaInfoTrack =  NULL;

   BDBG_ASSERT( NULL != pMediaInfoStream );
   BDBG_ASSERT( NULL != pTrack );
   BDBG_ASSERT( NULL != pStream );

   pMediaInfoTrack = B_Os_Calloc(1,sizeof(BIP_MediaInfoTrack));
   BIP_CHECK_GOTO(( pMediaInfoTrack!=NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfoTrack)  ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

   /* Populate pMediaInfoTrack from pTrack.*/
   populateMediaInfoTrack(pMediaInfoTrack , pTrack, pStream);
   BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "populateMediaInfoTrack failed" ), error, rc, rc );

   /* Adding track to track list. */
   BIP_MEDIAINFO_TRACKLIST_INSERT(&pMediaInfoStream->pFirstTrackInfoForStream, pMediaInfoTrack);
   /* Track added to track list.*/

   /* if program Number != 0 , then create then add this track to a trackGroup.*/
   if(pTrack->program != 0 && pStream->nprograms >= 1)
   {
       rc = addMediaInfoTrackToTrackGroup(pMediaInfoStream, pMediaInfoTrack, pTrack->program);
       BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addMediaInfoTrackToTrackGroup failed" ), error, rc, rc );
   }
   pMediaInfoStream->numberOfTracks++;

error:
    return rc;
}

static BIP_Status addPmtPidToMediaInfoTrackGroup(
        BIP_MediaInfoStream  *pMediaInfoStream,
        unsigned programNumber,
        unsigned pmtPid
        )
{
    BIP_Status       rc = BIP_SUCCESS;
    BIP_MediaInfoTrackGroup *pTrackGroup = NULL;

    /* first get the trackGroup for the specified groupId*/
    BIP_MEDIAINFO_GET_TRACKGROUP_FOR_GROUPID(&pMediaInfoStream->pFirstTrackGroupInfo, pTrackGroup, programNumber, &pMediaInfoStream->numberOfTrackGroups);
    pTrackGroup->type.Ts.pmtPid = pmtPid;
error:
    return rc;
}

static BIP_Status BIP_MediaInfo_GenerateBipMediaInfoFromBMediaStream(
     const bmedia_probe_stream *pStream,
     BIP_MediaInfoHandle hMediaInfo
     )
{
    BIP_Status rc    = BIP_SUCCESS;
    const bmedia_probe_track  *pTrack  = NULL;

    hMediaInfo->mediaInfoStream.durationInMs = pStream->duration;
    hMediaInfo->mediaInfoStream.maxBitRate = pStream->max_bitrate;

    hMediaInfo->mediaInfoStream.pMediaFileAbsolutePathname = BIP_String_GetString(hMediaInfo->hAbsoluteMediaPath);

    hMediaInfo->mediaInfoStream.transportType = BIP_MediaInfo_ConvertBmediaMpegTypeToNexus( pStream->type );

    if (( hMediaInfo->mediaInfoStream.transportType == NEXUS_TransportType_eTs ) && ((((bmpeg2ts_probe_stream *)pStream )->pkt_len ) == 192 ))
    {
        BDBG_MSG((BIP_MSG_PRE_FMT "TTS Stream\n" BIP_MSG_PRE_ARG ));
        hMediaInfo->mediaInfoStream.transportTimeStampEnabled = true;
    }
    else
    {
        hMediaInfo->mediaInfoStream.transportTimeStampEnabled = false;
    }

    /* Now get track_Group and track related information.*/
    /* First Iteration store Programs in  a List */
    for (pTrack = BLST_SQ_FIRST( &pStream->tracks ); pTrack; pTrack = BLST_SQ_NEXT( pTrack, link ))
    {
        switch (pTrack->type)
        {
            case bmedia_track_type_audio:
            case bmedia_track_type_video:
            case bmedia_track_type_pcr:
            {
                /* We won't do any validation check on codec (whether it is unknown format).
                We collect all the tracks and then app decide whta it wants to do. */
                rc = createMediaInfoTrack( &(hMediaInfo->mediaInfoStream) , pStream, pTrack );
                BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "createMediaInfoTrack failed" ), error, rc, rc );

                break;
            }
            case bmedia_track_type_other:
            {
                if((pStream->type == bstream_mpeg_type_ts) && (pStream->probe_id == BMPEG2TS_PSI_PROBE_ID || pStream->probe_id == BMPEG2TS192_PSI_PROBE_ID))
                {
                    switch( ((bmpeg2ts_psi_probe_track *) pTrack)->transport_type)
                    {
                        case bmpeg2ts_psi_transport_dvb_subtitles:
                        {
                            break;
                        }
                        case bmpeg2ts_psi_transport_teletext:
                        {
                            break;
                        }
                        case bmpeg2ts_psi_transport_pmt:
                        {
                            /* Add pmt pid to the track group if the track group is already created else create a track group and add pmt pid to it.*/
                           rc =  addPmtPidToMediaInfoTrackGroup(&(hMediaInfo->mediaInfoStream),
                                                  pTrack->program,
                                                  pTrack->number
                                                  );
                           BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addPmtPidToMediaInfoTrackGroup failed" ), error, rc, rc );
                           break;
                        }
                        case bmpeg2ts_psi_transport_pat:
                        {
                            break;
                        }
                        case bmpeg2ts_psi_transport_pcr:
                        {
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }/* end of switch */
                }/* end of if stream type check*/
            }/* end of case bmedia_track_type_other:*/
        }/*switch (pTrack->type)*/
    }/* end of for loop */

error:
return rc;
}

static BIP_Status BIP_MediaInfo_ProbeFileForMediaInfo(
    const char *fileName,
    BIP_MediaInfoHandle hMediaInfo
    )
{
    BIP_Status  rc    = BIP_SUCCESS;
    bmedia_probe_t             probe = NULL;
    bmedia_probe_config        probe_config;
    const bmedia_probe_stream *pStream = NULL;
    int             fd_int            = -1;
    bfile_io_read_t fd                = NULL;
    struct stat     fileStats;
    FILE           *fp         = NULL;

    BDBG_ENTER(BIP_MediaInfo_ProbeFileForMediaInfo);

    BDBG_ASSERT(hMediaInfo);
    BDBG_OBJECT_ASSERT(hMediaInfo,BIP_MediaInfo);

    fd_int = open( fileName, O_RDONLY | O_LARGEFILE );
    if (fd_int >= 0)
    {
        fp = fdopen( fd_int, "r" );
    }
    BIP_CHECK_GOTO((( fd_int>= 0 ) && fp ), ( "failed to open file (%s), fd %d, fp %p, errno %d\n", fileName, fd_int, fp, errno ), error, BIP_ERR_MEDIA_INFO_BAD_MEDIA_PATH, rc );
    BIP_CHECK_GOTO(( !( fstat( fd_int, &fileStats ))  && ( fileStats.st_size > 0 )), ( "Can't obtain file stats info on media file (%s) %d\n", fileName, errno ), error, BIP_ERR_MEDIA_INFO_BAD_MEDIA_PATH, rc );

    probe = bmedia_probe_create();
    BIP_CHECK_GOTO(( probe!=NULL ), ( "Can't create a Media probe object for parsing file %s \n", fileName ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    fd = bfile_stdio_read_attach( fp );

    /* we want to set the hAbsoluteMediaPath and mediaInfoStream.contentLength even before probe since this informations will be valid for an unknown container type file, that is for any file. */
    rc = BIP_String_StrcpyPrintf(hMediaInfo->hAbsoluteMediaPath, fileName);
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_String_StrcpyPrintf failed "), error, BIP_INF_MEDIA_INFO_UNKNOWN_CONTAINER_TYPE, rc );

    hMediaInfo->sizeInBytes =  fileStats.st_size;

    bmedia_probe_default_cfg( &probe_config );
    probe_config.file_name = fileName;
    probe_config.type      = bstream_mpeg_type_unknown;
    probe_config.min_probe_request = hMediaInfo->createSettings.psiAcquireProbeSizeInBytes;
    pStream = bmedia_probe_parse( probe, fd, &probe_config );
    if(pStream == NULL)
    {
        hMediaInfo->mediaInfoType = BIP_MediaInfoType_eUnknown;
        BDBG_MSG((BIP_MSG_PRE_FMT "-----------hMediaInfo->mediaInfoType = |%d|" BIP_MSG_PRE_ARG, hMediaInfo->mediaInfoType));
    }
    BIP_CHECK_GOTO(( pStream !=NULL ), ( "Media probe can't parse stream |%s|", fileName ), error, BIP_INF_MEDIA_INFO_UNKNOWN_CONTAINER_TYPE, rc );
    hMediaInfo->mediaInfoStream.contentLength =  fileStats.st_size;

    rc = BIP_MediaInfo_GenerateBipMediaInfoFromBMediaStream(pStream, hMediaInfo );
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "createMediaInfoTrack failed" ), error, rc, rc );

error:
    if(probe)
    {
        if (pStream) {bmedia_probe_stream_free( probe, pStream ); }
        if (fd) {bfile_stdio_read_detach( fd ); }
        bmedia_probe_destroy( probe );
    }

    if (fp)
    {fclose( fp ); }
    else if (fd_int >= 0)
    {close( fd_int ); }

    BDBG_LEAVE(BIP_MediaInfo_ProbeFileForMediaInfo);
    return rc;
}

static BIP_Status addAudioTrackToXmlTree(
    BIP_MediaInfoAudioTrack *pAudioTrack,
    BIP_XmlElement          xmlElemParent,
    NEXUS_TransportType     transportType
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_XmlElement xmlElemChild = NULL;

    BDBG_ASSERT(pAudioTrack);

    xmlElemChild = BIP_XmlElem_Create(xmlElemParent, BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_AUDIO);
    BIP_CHECK_GOTO(( xmlElemChild !=NULL ), ("BIP_XmlElem_Create failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    if(BIP_ToStr_NEXUS_AudioCodec( pAudioTrack->codec)) {
        BIP_XmlElem_AddAttr(xmlElemChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_CODEC, BIP_ToStr_NEXUS_AudioCodec( pAudioTrack->codec) );
    }
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_CHANNELCOUNT, pAudioTrack->channelCount);
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_SAMPLESIZE, pAudioTrack->sampleSize);
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_BITRATE, pAudioTrack->bitrate);
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_SAMPLERATE, pAudioTrack->sampleRate);

    if(pAudioTrack->pLanguage)
    {
        BIP_XmlElem_AddAttr(xmlElemChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_LANGUAGE, pAudioTrack->pLanguage);
    }

    BIP_XmlElem_AddAttrBool(xmlElemChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_SESSION_ENABLED, pAudioTrack->hlsSessionEnabled);

    /* Now for container specific info create a subchild.*/
    if(transportType == NEXUS_TransportType_eTs && pAudioTrack->hlsSessionEnabled == false)
    {
        BIP_XmlElement xmlElemSubChild = NULL;
        xmlElemSubChild = BIP_XmlElem_Create(xmlElemChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MPEGTS);
        BIP_CHECK_GOTO(( xmlElemSubChild !=NULL ), ("BIP_XmlElem_Create failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        BIP_XmlElem_AddAttrUnsigned(xmlElemSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MPEGTS_AUDIOTYPE, pAudioTrack->containerSpecificInfo.mpegTs.audioType);
    }
    if( pAudioTrack->hlsSessionEnabled == true)
    {
        BIP_XmlElement xmlElemSubChild = NULL;
        xmlElemSubChild = BIP_XmlElem_Create(xmlElemChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS);
        BIP_CHECK_GOTO(( xmlElemSubChild !=NULL ), ("BIP_XmlElem_Create failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        BIP_XmlElem_AddAttrBool(xmlElemSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_DEFAULTAUDIOFLAG, pAudioTrack->containerSpecificInfo.hls.defaultAudio);
        /*This pid addition only signed since this is coming as int from B_PlaybackIpPsiInfo structure's hlsAltAudioRenditionInfo member.*/
        /*All this inconsistency will be gone when we do hls probing in a propre way. */
        BIP_XmlElem_AddAttrInt(xmlElemSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_PID, pAudioTrack->containerSpecificInfo.hls.hlsAudioPid);

        if(pAudioTrack->containerSpecificInfo.hls.pHlsLanguageCode)
        {
            BIP_XmlElem_AddAttr(xmlElemSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_LANGUAGE_CODE, pAudioTrack->containerSpecificInfo.hls.pHlsLanguageCode);
        }

        if(pAudioTrack->containerSpecificInfo.hls.pLanguageName)
        {
            BIP_XmlElem_AddAttr(xmlElemSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_LANGUAGE_DESCRIPTION, pAudioTrack->containerSpecificInfo.hls.pLanguageName);
        }

        BIP_XmlElem_AddAttrBool(xmlElemSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_REQUIRED_2ND_PLAYPUMP, pAudioTrack->containerSpecificInfo.hls.requiresSecondPlaypumForAudio);

        if(pAudioTrack->containerSpecificInfo.hls.pGroupId)
        {
            BIP_XmlElem_AddAttr(xmlElemSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_GROUP_ID, pAudioTrack->containerSpecificInfo.hls.pGroupId);
        }

        if(BIP_ToStr_NEXUS_TransportType(pAudioTrack->containerSpecificInfo.hls.hlsExtraAudioSpecificContainerType))
        {
            BIP_XmlElem_AddAttr(
                xmlElemSubChild,
                BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_EXTRA_AUDIO_SPECIFIC_CONTAINER_TYPE,
                BIP_ToStr_NEXUS_TransportType(pAudioTrack->containerSpecificInfo.hls.hlsExtraAudioSpecificContainerType)
                );
        }
    }

    /* Now for codec specifc info create a subchild.*/
    if(pAudioTrack->codec == NEXUS_AudioCodec_eAc3 && pAudioTrack->descriptor.ac3.bsmodValid)
    {
        BIP_XmlElement xmlElemSubChild = NULL;
        xmlElemSubChild = BIP_XmlElem_Create(xmlElemChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_AC3);
        BIP_CHECK_GOTO(( xmlElemSubChild !=NULL ), ("BIP_XmlElem_Create failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        BIP_XmlElem_AddAttrUnsigned(xmlElemSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_AC3_BSMOD, pAudioTrack->descriptor.ac3.bsmod);
    }

error:
    return rc;
}

static BIP_Status addVideoTrackToXmlTree(
    BIP_MediaInfoVideoTrack *pVideoTrack,
    BIP_XmlElement xmlElemParent
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_XmlElement xmlElemChild = NULL;

    BDBG_ASSERT(pVideoTrack);

    xmlElemChild = BIP_XmlElem_Create(xmlElemParent, BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_VIDEO);
    BIP_CHECK_GOTO(( xmlElemChild !=NULL ), ("BIP_XmlElem_Create failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    if(BIP_ToStr_NEXUS_VideoCodec(pVideoTrack->codec)) {
        BIP_XmlElem_AddAttr(xmlElemChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_CODEC, BIP_ToStr_NEXUS_VideoCodec(pVideoTrack->codec));
    }
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_WIDTH, pVideoTrack->width);
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_HEIGHT, pVideoTrack->height);
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_BITRATE, pVideoTrack->bitrate);
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_FRAMERATE, pVideoTrack->frameRate);
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_COLORDEPTH, pVideoTrack->colorDepth);

error:
    return rc;
}

static BIP_Status addTrackToXmlTree(
    BIP_MediaInfoTrack *pTrack,
    BIP_XmlElement xmlElemParent,
    NEXUS_TransportType transportType
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_XmlElement xmlElemChild = NULL;

    xmlElemChild = BIP_XmlElem_Create(xmlElemParent, BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK);
    BIP_CHECK_GOTO(( xmlElemChild !=NULL ), ("BIP_XmlElem_Create failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    if(BIP_ToStr_BIP_MediaInfoTrackType(pTrack->trackType)) {
        BIP_XmlElem_AddAttr(xmlElemChild, BIP_MEDIAINFO_TRK_XML_ATT_TYPE, BIP_ToStr_BIP_MediaInfoTrackType(pTrack->trackType));
    }
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_XML_ATT_ID, pTrack->trackId);
    BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_XML_ATT_PARSED_PAYLOAD, pTrack->parsedPayload);

    #if 0
    /* TODO: Once gary updates the header file we will incorporate this changes.*/
    if(NEXUS_TransportType_eTs == pTrack->type)
    {
        /*rc = addTsTrackToXmlTree(xmlElemChild);*/
        BIP_XmlElement xmlElemSubChild = NULL;

        xmlElemSubChild = BIP_XmlElem_Create(xmlElemChild, BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_TS);

        BIP_XmlElem_AddAttrUnsigned(xmlElemSubChild, BIP_MEDIAINFO_TRK_XML_ATT_PARSED_PAYLOAD, pTrack->parsedPayload);
    }
    #endif

    if(pTrack->trackType == BIP_MediaInfoTrackType_eAudio)
    {
        rc = addAudioTrackToXmlTree(&(pTrack->info.audio), xmlElemChild, transportType);
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addAudioTrackToXmlTree failed" ), error, rc, rc );
    }
    else if(pTrack->trackType == BIP_MediaInfoTrackType_eVideo)
    {
        rc = addVideoTrackToXmlTree(&(pTrack->info.video), xmlElemChild);
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addVideoTrackToXmlTree failed" ), error, rc, rc );
    }
error:
    return rc;
}

static BIP_Status addTracksInTrackGroupToXmlTree(
    BIP_MediaInfoTrackGroup *pCurrentTrackGroup,
    BIP_XmlElement xmlElemParent,
    NEXUS_TransportType transportType
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_MediaInfoTrack *pCurrentTrack = NULL;

    BDBG_ASSERT(pCurrentTrackGroup);

    pCurrentTrack = pCurrentTrackGroup->pFirstTrackForTrackGroup;
    while(pCurrentTrack)
    {
        rc = addTrackToXmlTree( pCurrentTrack, xmlElemParent, transportType );
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addTrackToXmlTree failed" ), error, rc, rc );
        pCurrentTrack = pCurrentTrack->pNextTrackForTrackGroup;
    }
error:
    return rc;
}

static BIP_Status addTrackGroupsToXmlTree(
    BIP_MediaInfoStream *pMediaInfoStream,
    BIP_XmlElement xmlElemParent
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_MediaInfoTrackGroup *pCurrentTrackGroup = NULL;
    BIP_XmlElement xmlElemChild = NULL;

    BDBG_ASSERT(pMediaInfoStream);

    pCurrentTrackGroup = pMediaInfoStream->pFirstTrackGroupInfo;
    while(pCurrentTrackGroup)
    {
        /* Init xmlElemChild to NULL*/
        xmlElemChild = NULL;
        xmlElemChild = BIP_XmlElem_Create(xmlElemParent, BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_GROUP);
        BIP_CHECK_GOTO(( xmlElemChild !=NULL ), ("BIP_XmlElem_Create failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_GROUP_XML_ATT_ID, pCurrentTrackGroup->trackGroupId);
        BIP_XmlElem_AddAttrUnsigned(xmlElemChild, BIP_MEDIAINFO_TRK_GROUP_XML_ATT_NUM_TRKS, pCurrentTrackGroup->numberOfTracks);

        /* TODO: This should also become a sub child.*/
        if(pCurrentTrackGroup->type.Ts.pmtPid)
        {
            BIP_XmlElement xmlElemSubChild = NULL;

            xmlElemSubChild = BIP_XmlElem_Create(xmlElemChild, BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_GROUP_TS);
            BIP_XmlElem_AddAttrUnsigned(xmlElemSubChild, BIP_MEDIAINFO_TRK_GROUP_XML_ATT_PMT_PID, pCurrentTrackGroup->type.Ts.pmtPid);
        }

        /* add tracks in present in the trackGroup */
        /* addTracksInTrackGroup(pTrackCurrentGroup, xmlElemChild);*/
        if (pCurrentTrackGroup->numberOfTracks)
        {
            rc = addTracksInTrackGroupToXmlTree(pCurrentTrackGroup, xmlElemChild, pMediaInfoStream->transportType);
            BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addTracksInTrackGroup failed" ), error, rc, rc );
        }

        pCurrentTrackGroup = pCurrentTrackGroup->pNextTrackGroup;
    }

error:
    return rc;
}

static BIP_Status addTracksNotInTrackGroupToXmlTree(
        BIP_MediaInfoStream *pMediaInfoStream,
        BIP_XmlElement xmlElemParent
        )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_MediaInfoTrack *pCurrentTrack = NULL;

    BDBG_ASSERT(pMediaInfoStream);

    pCurrentTrack = pMediaInfoStream->pFirstTrackInfoForStream;

    while(pCurrentTrack)
    {
        /* we add it only when we found that this tracks doesn't belong to any trackGroup.*/
        if(pCurrentTrack->pParentTrackGroup == NULL)
        {
            rc = addTrackToXmlTree( pCurrentTrack, xmlElemParent , pMediaInfoStream->transportType);
            BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addTrackToXmlTree failed" ), error, rc, rc );
        }
        pCurrentTrack = pCurrentTrack->pNextTrackForStream;
    }
error:
    return rc;
}

static BIP_Status addMediaInfoStreamToXmlTree(
    BIP_MediaInfoHandle hMediaInfo,
    BIP_XmlElement xmlElemParent
    )
{
    BIP_XmlElement xmlElemChild = NULL;
    BIP_MediaInfoStream *pMediaInfoStream = NULL;
    BIP_Status rc    = BIP_SUCCESS;

    BDBG_ASSERT(hMediaInfo);
    BDBG_OBJECT_ASSERT(hMediaInfo,BIP_MediaInfo);

    BDBG_ASSERT(xmlElemParent);

    pMediaInfoStream = &(hMediaInfo->mediaInfoStream);

    xmlElemChild = BIP_XmlElem_Create( xmlElemParent, BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_STREAM);
    BIP_CHECK_GOTO(( xmlElemChild !=NULL ), ("BIP_XmlElem_Create failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Now add stream specific attributes.*/
    if(BIP_ToStr_NEXUS_TransportType(pMediaInfoStream->transportType))
    {
        BIP_XmlElem_AddAttr(  xmlElemChild, BIP_MEDIAINFO_STREAM_XML_ATT_TYPE,BIP_ToStr_NEXUS_TransportType(pMediaInfoStream->transportType));
    }
    BIP_XmlElem_AddAttrUnsigned( xmlElemChild, BIP_MEDIAINFO_STREAM_XML_ATT_NUM_GROUPS,pMediaInfoStream->numberOfTrackGroups);
    BIP_XmlElem_AddAttrUnsigned( xmlElemChild, BIP_MEDIAINFO_STREAM_XML_ATT_NUM_TRKS,pMediaInfoStream->numberOfTracks);
    BIP_XmlElem_AddAttrUnsigned( xmlElemChild, BIP_MEDIAINFO_STREAM_XML_ATT_AVG_BITRATE,pMediaInfoStream->avgBitRate);
    BIP_XmlElem_AddAttrUnsigned( xmlElemChild, BIP_MEDIAINFO_STREAM_XML_ATT_MAX_BITRATE,pMediaInfoStream->maxBitRate);
    BIP_XmlElem_AddAttrUnsigned( xmlElemChild, BIP_MEDIAINFO_STREAM_XML_ATT_DURATION,pMediaInfoStream->durationInMs);
    BIP_XmlElem_AddAttrInt64( xmlElemChild, BIP_MEDIAINFO_STREAM_XML_ATT_CONTENTLENGTH,pMediaInfoStream->contentLength);
    BIP_XmlElem_AddAttrBool( xmlElemChild, BIP_MEDIAINFO_STREAM_XML_ATT_LIVECHANNEL,pMediaInfoStream->liveChannel);
    BIP_XmlElem_AddAttrBool( xmlElemChild, BIP_MEDIAINFO_STREAM_XML_ATT_TTS_ENABLED,pMediaInfoStream->transportTimeStampEnabled);

    BDBG_MSG((BIP_MSG_PRE_FMT " pMediaInfoStream->numberOfTrackGroups ------------------> %d" BIP_MSG_PRE_ARG, pMediaInfoStream->numberOfTrackGroups));
    /* If any track group present then addTrackGroupToXmlTree().*/
    if(pMediaInfoStream->numberOfTrackGroups )
    {
        rc = addTrackGroupsToXmlTree(pMediaInfoStream, xmlElemChild);
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addTrackGroupsToXmlTree failed" ), error, rc, rc );
    }

    rc = addTracksNotInTrackGroupToXmlTree(pMediaInfoStream, xmlElemChild);
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addTrackGroupsToXmlTree failed" ), error, rc, rc );

error:
    return rc;
}

/**********************************************
  * Create info file from the xml tree.
  *********************************************/
static BIP_Status BIP_MediaInfo_CreateInfoFile(
    const char                  *pInfoFileAbsolutePathName,
    BIP_XmlElement              xmlElemRoot
    )
{
    BIP_Status       rc = BIP_SUCCESS;
    FILE            *fp = NULL;

    BIP_StringHandle hDirectoryCheckName = NULL;
    char            *pLastSlash          = NULL;
    int errnoVal;

    BDBG_ASSERT(xmlElemRoot);
    BDBG_ASSERT(pInfoFileAbsolutePathName);

    {
        pLastSlash = rindex( pInfoFileAbsolutePathName, '/' );
        errnoVal = errno;
        BIP_CHECK_GOTO(( pLastSlash != NULL ), ( "Can't find parent dir! Is CWD valid? dir name: \"%s\": errno %d", pInfoFileAbsolutePathName, errnoVal), error, BIP_ERR_OS_CHECK_ERRNO, rc );

        hDirectoryCheckName = BIP_String_CreateFromCharN( pInfoFileAbsolutePathName, pLastSlash - pInfoFileAbsolutePathName);
        BDBG_MSG(( BIP_MSG_PRE_FMT "Directory Check Name %s" BIP_MSG_PRE_ARG, BIP_String_GetString( hDirectoryCheckName )));
        rc = BIP_File_MakeDirectoryPath( BIP_String_GetString( hDirectoryCheckName ));
        errnoVal = BIP_StatusToErrno(rc);
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS && errnoVal == 0 ), ( "Can't do a mkdir -p at  : \"%s\" errnoVal = %d ", BIP_String_GetString( hDirectoryCheckName ), errnoVal), error, rc, rc );
    }

    /* Now write the BIP_MediaInfo meta data into the media info file. */
    fp = fopen( pInfoFileAbsolutePathName, "w" );
    BIP_CHECK_GOTO((fp != NULL),
                   ("fopen() failed for: %s",  pInfoFileAbsolutePathName),
                   error, BIP_StatusFromErrno(errno), rc );

    BIP_XmlElem_WriteToFile( xmlElemRoot,fp,-1); /* -1 since we don't want any indentation.*/

error:
    if (hDirectoryCheckName)
    {
        BIP_String_Destroy( hDirectoryCheckName ); hDirectoryCheckName = NULL;
    }

    if (fp)         { fclose( fp );    }
    return( rc );
} /* BIP_MediaInfo_CreateInfoFile */


BIP_Status BIP_MediaInfo_CreateXmlTree(
    BIP_MediaInfoHandle hMediaInfo,
    const char *pInfoFileAbsolutePathName
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_XmlElement xmlElemRoot = NULL;

    BDBG_ASSERT(hMediaInfo);
    BDBG_OBJECT_ASSERT(hMediaInfo,BIP_MediaInfo);

    BDBG_MSG((BIP_MSG_PRE_FMT "Creating XmlTree from info file."BIP_MSG_PRE_ARG));

    xmlElemRoot = BIP_XmlElem_Create(NULL,BIP_MEDIAINFO_XML_TAG_MEDIA_INFO);
    BIP_CHECK_GOTO(( xmlElemRoot !=NULL ), ("BIP_XmlElem_Create failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    hMediaInfo->xmlElemRoot = xmlElemRoot;

    /* Add version number. */
    BIP_XmlElem_AddAttr(
            xmlElemRoot,
            BIP_MEDIAINFO_XML_ATT_VERSION,
            BIP_MEDIA_INFO_VERSION
            );

    /* Add attributes for media. hAbsoluteMediaPath will be NULL for tunerInput.*/
    if(NULL != hMediaInfo->hAbsoluteMediaPath )
    {
        BIP_XmlElem_AddAttr(
            xmlElemRoot,
            BIP_MEDIAINFO_XML_ATT_ABS_MEDIA_PATH,
            BIP_String_GetString(hMediaInfo->hAbsoluteMediaPath)
            );
    }

    BIP_XmlElem_AddAttrInt64( xmlElemRoot, BIP_MEDIAINFO_XML_TAG_SIZE_IN_BYTES,hMediaInfo->sizeInBytes);

    BIP_XmlElem_AddAttr( xmlElemRoot, BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TYPE,BIP_ToStr_BIP_MediaInfoType(hMediaInfo->mediaInfoType));

    if(hMediaInfo->mediaInfoType == BIP_MediaInfoType_eStream)
    {
        rc = addMediaInfoStreamToXmlTree(hMediaInfo, xmlElemRoot);
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addMediaInfoStreamToXmlTree failed" ), error, rc, rc );
    }

    rc = BIP_MediaInfo_CreateInfoFile(
            pInfoFileAbsolutePathName,
            xmlElemRoot
            );
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_CreateInfoFile failed" ), error, rc, rc );

error:
    return rc;
}

/**********************************************
  * Check whether info file prsent.
  *********************************************/
BIP_Status BIP_MediaInfo_CheckForValidInfoFile(
     const char *pInfoFileAbsolutePathName
     )
{
    BIP_Status rc    = BIP_SUCCESS;
    int fd = -1;
    struct stat infoFileStats;
    FILE    *fp = NULL;

    /* Check to see if this info file exists */
    if ((( fd = open( pInfoFileAbsolutePathName, O_RDONLY )) < 0 ) || (( fstat( fd, &infoFileStats ) == 0 ) && ( infoFileStats.st_size <= 0 )) )
    {
        /*BIP_CHECK_GOTO(( fd != -1 ), ( "InfoFile requested(%s) Does Not exist or is empty.", pInfoFileAbsolutePathName ), error, BIP_INF_NOT_AVAILABLE, rc );*/
        /* This need not be marked as an error case. Check with Gary whether this should be BDBG_WRN or BDBG_MSG.
        Caller will capture error message from this function and based on the calling context it will decode whether this is error or not.*/
        BDBG_MSG((BIP_MSG_PRE_FMT "InfoFile requested(%s) Does Not exist or is empty." BIP_MSG_PRE_ARG, pInfoFileAbsolutePathName));
        rc = BIP_INF_NOT_AVAILABLE;
        goto error;
    }

    /* If file has some size, then we will assume that info file is valid.TODO:later we can add acheck to check the last entry in info file to determine whether the file has valid data.Like StraemValid or psi valid marker. */
    fp = fdopen( fd, "r" );
    BIP_CHECK_GOTO((fp != NULL),
           ("fdopen() failed for: %s", pInfoFileAbsolutePathName),
           error, BIP_StatusFromErrno(errno), rc );

error:
    if (fp){ fclose( fp );}
    else if (fd>=0){ close( fd ); }
    return rc;
}

static void populateVideoTrackFromXmlTree(
        BIP_MediaInfoTrack *pCurrentTrack,
        BIP_XmlElement  xmlElemParent
        )
{
    BIP_XmlElement xmlElemCurrentChild = NULL;

    BDBG_ASSERT(pCurrentTrack);
    BDBG_ASSERT(xmlElemParent);

    xmlElemCurrentChild = BIP_XmlElem_FindNextChildSameTag(
                                  xmlElemParent,
                                  NULL,
                                  BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_VIDEO
                                  );

    if(xmlElemCurrentChild)
    {
        if(BIP_XmlElem_FindAttrValue(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_CODEC)) {
            pCurrentTrack->info.video.codec = BIP_StrTo_NEXUS_VideoCodec(BIP_XmlElem_FindAttrValue(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_CODEC));
        }
        pCurrentTrack->info.video.width = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_WIDTH, 0 );
        pCurrentTrack->info.video.height = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_HEIGHT, 0 );
        pCurrentTrack->info.video.bitrate = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_BITRATE, 0 );
        pCurrentTrack->info.video.frameRate = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_FRAMERATE, 0 );
        pCurrentTrack->info.video.colorDepth = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild, BIP_MEDIAINFO_TRK_VIDEO_XML_ATT_COLORDEPTH, 0 );
    }
    else
    {
        BDBG_WRN((BIP_MSG_PRE_FMT "XmlTree contains an empty Video track." BIP_MSG_PRE_ARG));
    }
}

static void populateAudioTrackFromXmlTree(
        BIP_MediaInfoTrack *pCurrentTrack,
        BIP_XmlElement  xmlElemParent
        )
{
    BIP_XmlElement xmlElemCurrentChild = NULL;

    BDBG_ASSERT(pCurrentTrack);
    BDBG_ASSERT(xmlElemParent);

    xmlElemCurrentChild = BIP_XmlElem_FindNextChildSameTag(
                                  xmlElemParent,
                                  NULL,
                                  BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_AUDIO
                                  );

    if(xmlElemCurrentChild)
    {
        const char *temp = NULL;
        if(BIP_XmlElem_FindAttrValue(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_CODEC)) {
            pCurrentTrack->info.audio.codec = BIP_StrTo_NEXUS_AudioCodec(BIP_XmlElem_FindAttrValue(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_CODEC));
        }
        pCurrentTrack->info.audio.channelCount = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_CHANNELCOUNT, 0 );
        pCurrentTrack->info.audio.sampleSize = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_SAMPLESIZE, 0 );
        pCurrentTrack->info.audio.bitrate = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_BITRATE, 0 );
        pCurrentTrack->info.audio.sampleRate = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_SAMPLERATE, 0 );

        temp = BIP_XmlElem_FindAttrValue(xmlElemCurrentChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_LANGUAGE);
        if(temp)
        {
            /* we don't use the BIP_XmlElem_FindAttrValue returned pointer directly ,
            we copy from that to track specific memory so that they are independent
            and trackStructure need not be a const pointer which it can't be ,
            since it may be populated from mediaInfo->track structure also.*/
            pCurrentTrack->info.audio.pLanguage = B_Os_Calloc(1,(strlen(temp)+1));
            memcpy( pCurrentTrack->info.audio.pLanguage, temp, (strlen(temp)+1)); /* copy valid data to request buffer */
        }

        pCurrentTrack->info.audio.hlsSessionEnabled = BIP_XmlElem_FindAttrValueBoolean(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_SESSION_ENABLED, false );


        if(pCurrentTrack->info.audio.hlsSessionEnabled)
        {
            BIP_XmlElement xmlElemCurrentSubChild = NULL;
            const char *languageName = NULL;
            const char *pGroupId = NULL;
            const char *pHlsLanguageCode = NULL;

             /* If not mpeg2Ts then check for hls */
            xmlElemCurrentSubChild = BIP_XmlElem_FindNextChildSameTag(
                                      xmlElemCurrentChild,
                                      NULL,
                                      BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS
                                      );

            if(xmlElemCurrentSubChild)
            {
                pCurrentTrack->info.audio.containerSpecificInfo.hls.defaultAudio = BIP_XmlElem_FindAttrValueBoolean(
                                                                                xmlElemCurrentSubChild,
                                                                                BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_DEFAULTAUDIOFLAG,
                                                                                false
                                                                                );
                pCurrentTrack->info.audio.containerSpecificInfo.hls.hlsAudioPid = BIP_XmlElem_FindAttrValueInt(
                                                                            xmlElemCurrentSubChild,
                                                                            BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_PID,
                                                                            0
                                                                            );

                pHlsLanguageCode = BIP_XmlElem_FindAttrValue(
                        xmlElemCurrentSubChild,
                        BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_LANGUAGE_CODE
                        );

                if(pHlsLanguageCode)
                {
                    pCurrentTrack->info.audio.containerSpecificInfo.hls.pHlsLanguageCode = B_Os_Calloc(1,(strlen(pHlsLanguageCode)+1));
                    memcpy( pCurrentTrack->info.audio.containerSpecificInfo.hls.pHlsLanguageCode, pHlsLanguageCode, (strlen(pHlsLanguageCode)+1));
                }

                languageName = BIP_XmlElem_FindAttrValue(
                                xmlElemCurrentSubChild,
                                BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_LANGUAGE_DESCRIPTION
                                );
                if(languageName)
                {
                    pCurrentTrack->info.audio.containerSpecificInfo.hls.pLanguageName = B_Os_Calloc(1,(strlen(languageName)+1));
                    memcpy( pCurrentTrack->info.audio.containerSpecificInfo.hls.pLanguageName, languageName, (strlen(languageName)+1)); /* copy valid data to request buffer */
                }
                pCurrentTrack->info.audio.containerSpecificInfo.hls.requiresSecondPlaypumForAudio = BIP_XmlElem_FindAttrValueBoolean(
                                                                                            xmlElemCurrentSubChild,
                                                                                            BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_REQUIRED_2ND_PLAYPUMP,
                                                                                            false
                                                                                            );

                pGroupId = BIP_XmlElem_FindAttrValue( xmlElemCurrentSubChild,BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_GROUP_ID );
                if(pGroupId)
                {
                    pCurrentTrack->info.audio.containerSpecificInfo.hls.pGroupId = B_Os_Calloc(1,(strlen(pGroupId)+1));
                    memcpy(pCurrentTrack->info.audio.containerSpecificInfo.hls.pGroupId, pGroupId, (strlen(pGroupId)+1));
                }

                if(BIP_XmlElem_FindAttrValue(xmlElemCurrentSubChild , BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_EXTRA_AUDIO_SPECIFIC_CONTAINER_TYPE ))
                {
                    pCurrentTrack->info.audio.containerSpecificInfo.hls.hlsExtraAudioSpecificContainerType = BIP_StrTo_NEXUS_TransportType(
                     BIP_XmlElem_FindAttrValue(xmlElemCurrentSubChild , BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_HLS_EXTRA_AUDIO_SPECIFIC_CONTAINER_TYPE )
                     );
                }
            }
        }
        else
        {
            BIP_XmlElement xmlElemCurrentSubChild = NULL;

            xmlElemCurrentSubChild = BIP_XmlElem_FindNextChildSameTag(
                                          xmlElemCurrentChild,
                                          NULL,
                                          BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MPEGTS
                                          );
            if(xmlElemCurrentSubChild)
            {
                /* Now check whether the value exist */
                if(BIP_XmlElem_FindAttrValue(xmlElemCurrentSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MPEGTS_AUDIOTYPE))
                {
                    pCurrentTrack->info.audio.containerSpecificInfo.mpegTs.audioType = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MPEGTS_AUDIOTYPE, 0);
                }
            }
        }

        /* Now check whether ac3 specific info is present. */
        if( pCurrentTrack->info.audio.codec == NEXUS_AudioCodec_eAc3 )
        {
            BIP_XmlElement xmlElemCurrentSubChild = NULL;

            /* If not mpeg2Ts then check for hls */
            xmlElemCurrentSubChild = BIP_XmlElem_FindNextChildSameTag(
                                  xmlElemCurrentChild,
                                  NULL,
                                  BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_AC3
                                  );

            if(xmlElemCurrentSubChild)
            {
                if(BIP_XmlElem_FindAttrValue(xmlElemCurrentSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_AC3_BSMOD))
                {
                    pCurrentTrack->info.audio.descriptor.ac3.bsmodValid = true;
                    pCurrentTrack->info.audio.descriptor.ac3.bsmod = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentSubChild, BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_AC3_BSMOD, 0);
                    BDBG_MSG((BIP_MSG_PRE_FMT "pCurrentTrack->info.audio.descriptor.ac3.bsmod ++++++++++=====>%d" BIP_MSG_PRE_ARG, pCurrentTrack->info.audio.descriptor.ac3.bsmod));
                }
            }
        }
#if 0
        /* TODO: We can add more ac3 descriptor data as and when needed.*/
        #define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MAINAUDIO       "mainAudio"
        #define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_MAINAUDIOID     "mainAudioId"
        #define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_ASVCFLAGS       "asvcflags"
        #define BIP_MEDIAINFO_TRK_AUDIO_XML_ATT_LANGUAGE2       "language2"
#endif
    }
    else
    {
        BDBG_WRN((BIP_MSG_PRE_FMT "XmlTree contains an empty Audio track." BIP_MSG_PRE_ARG));
    }
}

static BIP_Status createTrackFromXmlTree(
     BIP_MediaInfoStream *pMediaInfoStream,
     BIP_MediaInfoTrackGroup *pTrackGroup, /* Pass NULL if track doesn't belong to any trackGroup.*/
     BIP_XmlElement xmlElemCurrentChild
     )
{
    BIP_MediaInfoTrack *pCurrentTrack = NULL;
    BIP_Status rc    = BIP_SUCCESS;

    /* Create a new track and add it to BIP_TrackGroup list.*/
    pCurrentTrack = B_Os_Calloc(1, sizeof(BIP_MediaInfoTrack));
    BIP_CHECK_GOTO(( pCurrentTrack!=NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfoTrack", sizeof(BIP_MediaInfoTrack)  ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* if pTrackGroup is not NULL add the track to trackGroup.*/
    if(pTrackGroup)
    {
        BIP_MEDIAINFO_TRACKGROUP_INSERT_TRACK(&pTrackGroup->pFirstTrackForTrackGroup, pCurrentTrack);
        pCurrentTrack->pParentTrackGroup = pTrackGroup;
    }

    /* Adding track to track list. */
    BIP_MEDIAINFO_TRACKLIST_INSERT(&pMediaInfoStream->pFirstTrackInfoForStream, pCurrentTrack);
    /* Track added to track list.*/

    /* Populate track from the xml tree.*/
    pCurrentTrack->trackType = BIP_StrTo_BIP_MediaInfoTrackType(BIP_XmlElem_FindAttrValue(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_XML_ATT_TYPE));
    pCurrentTrack->trackId = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_XML_ATT_ID, 0 );
    pCurrentTrack->parsedPayload = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_XML_ATT_PARSED_PAYLOAD, 0 );

    if(BIP_MediaInfoTrackType_eAudio == pCurrentTrack->trackType)
    {
        populateAudioTrackFromXmlTree(pCurrentTrack, xmlElemCurrentChild);
    }
    else if(BIP_MediaInfoTrackType_eVideo == pCurrentTrack->trackType)
    {
        populateVideoTrackFromXmlTree(pCurrentTrack, xmlElemCurrentChild);
    }
error:
    return rc;
}

static BIP_Status createTracksForTrackGroupsFromXmlTree(
    BIP_MediaInfoStream *pMediaInfoStream,
    BIP_MediaInfoTrackGroup *pTrackGroup,
    BIP_XmlElement xmlElemParent
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_XmlElement xmlElemCurrentChild = NULL;

    BDBG_ASSERT(pTrackGroup);
    BDBG_ASSERT(xmlElemParent);

    xmlElemCurrentChild = BIP_XmlElem_FindNextChildSameTag(
                                  xmlElemParent,
                                  NULL,
                                  BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK
                                  );

    BIP_CHECK_GOTO((xmlElemCurrentChild != NULL),("BIP_XmlElem_FindChild Failed for the 1st %s element", BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK), error, BIP_ERR_INTERNAL, rc );

    while(xmlElemCurrentChild)
    {
        rc = createTrackFromXmlTree(pMediaInfoStream, pTrackGroup, xmlElemCurrentChild);
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "createTrackFromXmlTree failed "), error,rc , rc );

        xmlElemCurrentChild = BIP_XmlElem_FindNextChildSameTag(
                                  xmlElemParent,
                                  xmlElemCurrentChild,
                                  BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK
                                  );
    }
error:
    return rc;
}

static BIP_Status BIP_MediaInfo_CreateTrackGroupsFromXmlTree(
    BIP_MediaInfoStream *pMediaInfoStream,
    BIP_XmlElement     xmlElemCurrentChild
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_XmlElement xmlElemSubChild = NULL;

    BDBG_ASSERT(pMediaInfoStream);
    BDBG_ASSERT(xmlElemCurrentChild);

    if(xmlElemCurrentChild)
    {
       BIP_MediaInfoTrackGroup *pTrackGroup = NULL;
       /* Create a new track group and add it In BIP_MediaInfo tree.*/
       pTrackGroup = B_Os_Calloc(1, sizeof(BIP_MediaInfoTrackGroup));
       BIP_CHECK_GOTO(( pTrackGroup!=NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfoTrackGroup", sizeof(BIP_MediaInfoTrackGroup)  ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

       BIP_MEDIAINFO_GROUPLIST_INSERT(&pMediaInfoStream->pFirstTrackGroupInfo, pTrackGroup);

       /* Now pupulate every track group specific information.*/
       pTrackGroup->trackGroupId = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_GROUP_XML_ATT_ID, 0 );
       pTrackGroup->numberOfTracks = BIP_XmlElem_FindAttrValueUnsigned(xmlElemCurrentChild , BIP_MEDIAINFO_TRK_GROUP_XML_ATT_NUM_TRKS, 0 );

       {/* get the sub child if present */
           xmlElemSubChild = BIP_XmlElem_FindChild(xmlElemCurrentChild, BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_GROUP_TS);
           if(xmlElemSubChild)
           {
               pTrackGroup->type.Ts.pmtPid = BIP_XmlElem_FindAttrValueUnsigned(xmlElemSubChild , BIP_MEDIAINFO_TRK_GROUP_XML_ATT_PMT_PID, 0 );
           }
       }

       if(pTrackGroup->numberOfTracks)
       {
           rc =  createTracksForTrackGroupsFromXmlTree( pMediaInfoStream, pTrackGroup, xmlElemCurrentChild);
           BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "createTracksForTrackGroupsFromXmlTree failed "), error,rc , rc );
       }
    }
error:
    return rc;
}

static  BIP_Status BIP_MediaInfo_PopulateMediaInfoFromXmlTree(
    BIP_MediaInfoHandle    hMediaInfo,
    BIP_XmlElement xmlElemRoot
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_MediaInfoStream *pMediaInfoStream = NULL;
    BIP_XmlElement xmlElemChild = NULL;
    BIP_XmlElement xmlElemMediaInfo= NULL;
    const char * pMediaPathFromInfo = NULL;

    pMediaInfoStream = &(hMediaInfo->mediaInfoStream);

    /* Get the pointer to the MediaInfo object. */
    xmlElemMediaInfo = BIP_XmlElem_FindChild(xmlElemRoot , BIP_MEDIAINFO_XML_TAG_MEDIA_INFO);
    BIP_CHECK_GOTO((xmlElemMediaInfo != NULL),("BIP_XmlElem_FindChild Failed for %s tag", BIP_MEDIAINFO_XML_TAG_MEDIA_INFO), error, BIP_ERR_INTERNAL, rc );

    /* Check for version number. */
    if(strncmp(BIP_MEDIA_INFO_VERSION , BIP_XmlElem_FindAttrValue(xmlElemMediaInfo , BIP_MEDIAINFO_XML_ATT_VERSION ), sizeof(BIP_MEDIA_INFO_VERSION)))
    {
        /* Need to regenerate the info files.*/
        rc = BIP_INF_MEDIA_INFO_VERSION_MISMATCH;
        goto error;
    }

    pMediaPathFromInfo = BIP_XmlElem_FindAttrValue(xmlElemMediaInfo , BIP_MEDIAINFO_XML_ATT_ABS_MEDIA_PATH );

    if(pMediaPathFromInfo)
    {
        rc = BIP_String_StrcpyPrintf(hMediaInfo->hAbsoluteMediaPath, pMediaPathFromInfo);
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_String_StrcpyPrintf failed "), error, rc, rc );
    }

    hMediaInfo->sizeInBytes = BIP_XmlElem_FindAttrValueInt64(xmlElemMediaInfo, BIP_MEDIAINFO_XML_TAG_SIZE_IN_BYTES, 0);
    hMediaInfo->mediaInfoType = BIP_StrTo_BIP_MediaInfoType(BIP_XmlElem_FindAttrValue( xmlElemMediaInfo, BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TYPE));

    if(hMediaInfo->mediaInfoType != BIP_MediaInfoType_eStream)
    {
        BDBG_MSG((BIP_MSG_PRE_FMT "mediaInfoType is unknown" BIP_MSG_PRE_ARG));
        /* Nothing to be done no more information is avilable.*/
        goto error;
    }

#if 0
    /* TODO: The following check need to be added at some point after a discussion with Gary/Sanjeev */
    /* Now get the mediaInfo element and then get the path of the media, if path is already provided and that doesn't match with the info file specified path
    then mark it as an error, in that case info file may need an update.keep in mind that path may mismach since both path may have inequla number "/".*/
    xmlElemMedia = BIP_MediaInfo_FindChild(xmlElemParsed , BIP_MEDIAINFO_XML_TAG_MEDIA_INFO);
    mediaPathFromInfo = BIP_MediaInfo_FindAttrValue(xmlElem , pMediaFileAbsolutePathname );
#endif

    /* Now populate the mediaInfoStream strcuture.*/
    /* Reinitialize child element. */
    xmlElemChild = NULL;
    xmlElemChild = BIP_XmlElem_FindChild(xmlElemMediaInfo , BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_STREAM);
    BIP_CHECK_GOTO((xmlElemChild != NULL),("BIP_XmlElem_FindChild Failed for %s element", BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_STREAM), error, BIP_ERR_INTERNAL, rc );

    if(BIP_XmlElem_FindAttrValue(xmlElemChild , BIP_MEDIAINFO_STREAM_XML_ATT_TYPE ))
    {
        pMediaInfoStream->transportType = BIP_StrTo_NEXUS_TransportType(BIP_XmlElem_FindAttrValue(xmlElemChild , BIP_MEDIAINFO_STREAM_XML_ATT_TYPE ));
    }

    pMediaInfoStream->numberOfTrackGroups = BIP_XmlElem_FindAttrValueUnsigned(xmlElemChild , BIP_MEDIAINFO_STREAM_XML_ATT_NUM_GROUPS, 0 );
    pMediaInfoStream->numberOfTracks = BIP_XmlElem_FindAttrValueUnsigned(xmlElemChild , BIP_MEDIAINFO_STREAM_XML_ATT_NUM_TRKS, 0 );
    pMediaInfoStream->avgBitRate = BIP_XmlElem_FindAttrValueUnsigned(xmlElemChild , BIP_MEDIAINFO_STREAM_XML_ATT_AVG_BITRATE, 0 );
    pMediaInfoStream->maxBitRate = BIP_XmlElem_FindAttrValueUnsigned(xmlElemChild , BIP_MEDIAINFO_STREAM_XML_ATT_MAX_BITRATE, 0 );
    pMediaInfoStream->durationInMs = BIP_XmlElem_FindAttrValueUnsigned(xmlElemChild , BIP_MEDIAINFO_STREAM_XML_ATT_DURATION, 0 );
    pMediaInfoStream->contentLength = BIP_XmlElem_FindAttrValueInt64(xmlElemChild , BIP_MEDIAINFO_STREAM_XML_ATT_CONTENTLENGTH, 0 );
    pMediaInfoStream->liveChannel = BIP_XmlElem_FindAttrValueBoolean(xmlElemChild , BIP_MEDIAINFO_STREAM_XML_ATT_LIVECHANNEL, false );
    pMediaInfoStream->transportTimeStampEnabled = BIP_XmlElem_FindAttrValueBoolean(xmlElemChild , BIP_MEDIAINFO_STREAM_XML_ATT_TTS_ENABLED , false );

    /* Now get the next child element and add */
    {
        BIP_XmlElement xmlElemSubChild = NULL;
        const char *pTagName = NULL;
        xmlElemSubChild = BIP_XmlElem_FirstChild( xmlElemChild );
        BIP_CHECK_GOTO(( xmlElemSubChild != NULL ), ( "BIP_XmlElem_FirstChild for %s failed", BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_STREAM ), error, rc, rc );

        while(xmlElemSubChild)
        {
            pTagName = BIP_XmlElem_GetTag(xmlElemSubChild);

            if(!strcasecmp(BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK_GROUP, pTagName))
            {
                /* Create and add track groups.*/
                BIP_MediaInfo_CreateTrackGroupsFromXmlTree( pMediaInfoStream,xmlElemSubChild);
            }
            else if(!strcasecmp(BIP_MEDIAINFO_XML_TAG_MEDIA_INFO_TRK, pTagName))
            {
                /* Create and add tracks not in any trackGroups.*/
                rc = createTrackFromXmlTree(pMediaInfoStream, NULL, xmlElemSubChild);
                BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "createTrackFromXmlTree failed "), error,rc , rc );
            }
            xmlElemSubChild = BIP_XmlElem_NextChild(xmlElemChild);
        }
    }
error:
    return rc;
}


/**********************************************************************************
 * Retrive media info meta data from info file. Info file is validated before,
 * calling this function ,so no need of any more validation inside this function
 **********************************************************************************/
static BIP_Status BIP_MediaInfo_RetriveMetaDataFromInfoFile(
                     const char *pInfoFileAbsolutePathName,
                     BIP_MediaInfoHandle    hMediaInfo
                     )
{
    BIP_Status rc    = BIP_SUCCESS;
    FILE      *fp    = NULL;
    int        osRc;
    int        fd = -1;
    char *pReadBuffer = NULL;
    BIP_XmlElement xmlElemParsed = NULL;
    struct stat infoFileStats;
    size_t  fileSize = 0;

    BDBG_ASSERT(hMediaInfo);
    BDBG_OBJECT_ASSERT(hMediaInfo,BIP_MediaInfo);

    fd = open( pInfoFileAbsolutePathName, O_RDONLY );
    BIP_CHECK_GOTO((fd >= 0),
                   (" Can't find or parse this info file. Check path to directory containing info files : %s ", pInfoFileAbsolutePathName),
                   error, BIP_StatusFromErrno(errno), rc );

    fp = fdopen( fd, "r" );
    BIP_CHECK_GOTO((fp != NULL),
                   ("fdopen() failed for: %s", pInfoFileAbsolutePathName),
                   error, BIP_StatusFromErrno(errno), rc );

    /* Here we call fstat not mainly for validation but to determine the file size.*/
    osRc = fstat( fd, &infoFileStats );
    BIP_CHECK_GOTO((osRc == 0),
                   ("fstat() failed for: %s", pInfoFileAbsolutePathName),
                   error, BIP_StatusFromErrno(errno), rc );

    BIP_CHECK_GOTO((infoFileStats.st_size > 0),
                   (" Info file is of zero size : %s ", pInfoFileAbsolutePathName ),
                   error, BIP_ERR_MEDIA_INFO_BAD_INFOFILE_PATH, rc );

    pReadBuffer = B_Os_Calloc( 1, (infoFileStats.st_size+1));
    BIP_CHECK_GOTO(( pReadBuffer != NULL ), ("B_Os_Calloc failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    fileSize = fread(pReadBuffer, 1, infoFileStats.st_size , fp);
    BIP_CHECK_GOTO(( fileSize == (size_t)infoFileStats.st_size ), ("fread frim info file has an error:|fileSize != infoFileStats.st_size|"), error, BIP_ERR_INTERNAL, rc );

    /* Add the null character for end of string.*/
    pReadBuffer[infoFileStats.st_size] = '\0';

    /* Parse the info file and create the xml tree.*/
    xmlElemParsed = BIP_Xml_Create(pReadBuffer);
    BIP_CHECK_GOTO(( xmlElemParsed != NULL ), ("BIP_Xml_Create failed"), error, BIP_ERR_INTERNAL, rc );

    rc = BIP_MediaInfo_PopulateMediaInfoFromXmlTree( hMediaInfo, xmlElemParsed );
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS || rc == BIP_INF_MEDIA_INFO_VERSION_MISMATCH), ( "BIP_MediaInfo_PopulateMediaInfoFromXmlTree failed" ), error, rc, rc );

error:
    if (fp){ fclose( fp );}
    else if (fd >= 0){ close( fd ); }

    if(pReadBuffer)
    {
        B_Os_Free(pReadBuffer);
    }
    return rc;
}

/**********************************************************/
/** ProbeTuner specific sub functions and data structure. **/
/**********************************************************/
#ifndef STREAMER_CABLECARD_SUPPORT
static void tsPsi_procProgDescriptors(
    const uint8_t  *p_bfr,
    unsigned        bfrSize,
    PROGRAM_INFO_T *progInfo
    )
{
    int               i;
    TS_PSI_descriptor descriptor;

    for (i = 0, descriptor = TS_PMT_getDescriptor( p_bfr, bfrSize, i );
         descriptor != NULL;
         i++, descriptor = TS_PMT_getDescriptor( p_bfr, bfrSize, i ))
    {
        switch (descriptor[0])
        {
            case TS_PSI_DT_CA:
            {
                progInfo->ca_pid = (( descriptor[4] & 0x1F ) << 8 ) + descriptor[5];
                break;
            }

            default:
            {
                break;
            }
        } /* switch */
    }
} /* tsPsi_procProgDescriptors */

static void tsPsi_procStreamDescriptors(
    const uint8_t *p_bfr,
    unsigned       bfrSize,
    int            streamNum,
    EPID          *ePidData
    )
{
    int               i;
    TS_PSI_descriptor descriptor;

    for (i = 0, descriptor = TS_PMT_getStreamDescriptor( p_bfr, bfrSize, streamNum, i );
         descriptor != NULL;
         i++, descriptor = TS_PMT_getStreamDescriptor( p_bfr, bfrSize, streamNum, i ))
    {
        switch (descriptor[0])
        {
            case TS_PSI_DT_CA:
            {
                ePidData->ca_pid = (( descriptor[4] & 0x1F ) << 8 ) + descriptor[5];
                break;
            }

            default:
            {
                break;
            }
        } /* switch */
    }
} /* tsPsi_procStreamDescriptors */

#endif /* ifndef STREAMER_CABLECARD_SUPPORT */

#define NUM_PID_CHANNELS           4
#define MAX_PROGRAMS_PER_FREQUENCY 24

typedef enum {
    IpStreamerSrc_eIp,       /* For streaming out content coming from IP frontend */
    IpStreamerSrc_eQam,      /* For streaming out content coming from QAM frontend */
    IpStreamerSrc_eVsb,      /* For streaming out content coming from VSB frontend (off-air) */
    IpStreamerSrc_eStreamer, /* For streaming out content coming from Streamer input */
    IpStreamerSrc_eFile,     /* For streaming out pre-recorded content from local disk */
    IpStreamerSrc_eSat,      /* For streaming out content coming from Satellite frontend */
    IpStreamerSrc_eHdmi,     /* For streaming out encoded content coming from HDMI in (BlueRay player) */
    IpStreamerSrc_eOfdm,     /* For streaming out encoded content coming from OFDM-capable frontends */
    IpStreamerSrc_eMax       /* Max allowed enum */
} IpStreamerSrc;

typedef struct psiCollectionDataType
{
    IpStreamerSrc        srcType;
    NEXUS_ParserBand     parserBand;
    B_EventHandle        signalLockedEvent;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_FilePlayHandle file;
#ifdef NEXUS_HAS_PLAYBACK
    B_PlaybackIpHandle playbackIp;
#endif
    bool live;
    struct {
        uint16_t               num;
        NEXUS_PidChannelHandle channel;
    } pid[NUM_PID_CHANNELS];
} psiCollectionDataType, *pPsiCollectionDataType;

/* struct used to keep track of the si callback we must use, to notify
   the si applib that it's previously requested data is now available */
typedef struct siRequest_t
{
    B_PSIP_DataReadyCallback callback;
    void                    *callbackParam;
} siRequest_t;

/* we only have one filterHandle (i.e. msg), so we only have to keep track
   of the current request from si (specifically, the si "data ready"
   callback and associated param) */
static siRequest_t g_siRequest;

/* nexus message "data ready" callback - forward notification to si lib */
static void DataReady(
    void *context,
    int   param
    )
{
    BSTD_UNUSED( context );
    BSTD_UNUSED( param );
    BDBG_MSG((BIP_MSG_PRE_FMT "Nexus message data ready callback " BIP_MSG_PRE_ARG));

    if (NULL != g_siRequest.callback)
    {
        /* printf("in DataReady callback - forward notification to psip lib\n");*/
        g_siRequest.callback( g_siRequest.callbackParam );
    }
}

/* message "data ready" callback which is called by si when our requested data
   has been processed and is ready for retrieval */
static void SiDataReady(
    B_Error status,
    void   *context
    )
{
    /* printf("in APP DataReady callback - psip lib has finished processing data\n");*/
    if (B_ERROR_SUCCESS == status)
    {
        B_Event_Set((B_EventHandle)context );
    }
    else
    {
        BDBG_MSG((BIP_MSG_PRE_FMT "Problem receiving data from api call - error code:%d\n" BIP_MSG_PRE_ARG, status ));
        /* set event so our test app can continue... */
        B_Event_Set((B_EventHandle)context );
    }
}

/* gets a pidchannel - nexus handles pid channel reuse automatically */
static NEXUS_PidChannelHandle openPidChannel(
    psiCollectionDataType *pCollectionData,
    uint16_t               pid
    )
{
    int i = 0;
    NEXUS_PidChannelSettings             pidChannelSettings;
    NEXUS_PlaypumpOpenPidChannelSettings playpumpPidChannelSettings;

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings( &playpumpPidChannelSettings );

    playpumpPidChannelSettings.pidSettings.requireMessageBuffer = true;
    pidChannelSettings.requireMessageBuffer = true;
    NEXUS_PidChannel_GetDefaultSettings( &pidChannelSettings );

    for (i = 0; i < NUM_PID_CHANNELS; i++)
    {
        if (NULL == pCollectionData->pid[i].channel)
        {
            BDBG_MSG((BIP_MSG_PRE_FMT "Open pidChannel for pid:0x%04x\n" BIP_MSG_PRE_ARG, pid ));
            pCollectionData->pid[i].num = pid;
            if (pCollectionData->srcType == IpStreamerSrc_eIp)
            {
                pCollectionData->pid[i].channel =
                    NEXUS_Playpump_OpenPidChannel( pCollectionData->playpump, pid, &playpumpPidChannelSettings );
            }
            else
            {
                pCollectionData->pid[i].channel =
                    NEXUS_PidChannel_Open( pCollectionData->parserBand, pid, &pidChannelSettings );
            }
            return( pCollectionData->pid[i].channel );
        }
    }

    if (i == NUM_PID_CHANNELS)
    {
        BDBG_ERR(( "failed to open pid channel:0x%04x - not enough storage space in pCollectionData!\n", pid ));
    }

    return( NULL );
} /* openPidChannel */

/* utility function to convert si applib message filter to nexus message filter
   assumes pNexusFilter has already been properly initialized with default values */
static void cpyFilter(
    NEXUS_MessageFilter *pNexusFilter,
    B_PSIP_Filter       *pSiFilter
    )
{
    int i = 0;

    for (i = 0; i < BIP_MIN( B_PSIP_FILTER_SIZE, NEXUS_MESSAGE_FILTER_SIZE ); i++)
    {
        if (i == 2)
        {
            /* WARNING: will not filter message byte 2! */
            continue;
        }
        else if (i >= 2)
        {
            /* adjust nexus index see NEXUS_MessageFilter in nexus_message.h */
            pNexusFilter->mask[i-1]        = pSiFilter->mask[i];
            pNexusFilter->coefficient[i-1] = pSiFilter->coef[i];
            pNexusFilter->exclusion[i-1]   = pSiFilter->excl[i];
        }
        else
        {
            pNexusFilter->mask[i]        = pSiFilter->mask[i];
            pNexusFilter->coefficient[i] = pSiFilter->coef[i];
            pNexusFilter->exclusion[i]   = pSiFilter->excl[i];
        }
    }
} /* cpyFilter */

static void startMessageFilter(
    psiCollectionDataType    *pCollectionData,
    B_PSIP_CollectionRequest *pRequest,
    NEXUS_PidChannelHandle    pidChannel
    )
{
    NEXUS_MessageHandle        msg;
    NEXUS_MessageStartSettings msgStartSettings;

    BSTD_UNUSED( pCollectionData );
    msg = (NEXUS_MessageHandle)pRequest->filterHandle;
    /* printf("StartMessageFilter\n");*/

    NEXUS_Message_GetDefaultStartSettings( msg, &msgStartSettings );
    msgStartSettings.bufferMode = NEXUS_MessageBufferMode_eOneMessage;
    msgStartSettings.pidChannel = pidChannel;
    cpyFilter( &msgStartSettings.filter, &pRequest->filter );

    NEXUS_Message_Start( msg, &msgStartSettings );
    NEXUS_StartCallbacks( msg );
} /* startMessageFilter */

static void stopMessageFilter(
    psiCollectionDataType    *pCollectionData,
    B_PSIP_CollectionRequest *pRequest
    )
{
    NEXUS_MessageHandle msg;

    BSTD_UNUSED( pCollectionData );

    /* printf("StopMessageFilter\n"); */
    msg = (NEXUS_MessageHandle)pRequest->filterHandle;
    NEXUS_StopCallbacks( msg );
    NEXUS_Message_Stop( msg );
}

/* closes a previously opened pid channel */
static void closePidChannel(
    psiCollectionDataType *pCollectionData,
    uint16_t               pid
    )
{
    int  i     = 0;
    bool found = false;

    for (i = 0; i < NUM_PID_CHANNELS; i++)
    {
        /* find pid to close */
        if (( pCollectionData->pid[i].num == pid ) &&
            ( pCollectionData->pid[i].channel != NULL ))
        {
            BDBG_MSG((BIP_MSG_PRE_FMT "close pidChannel for pid:0x%04x\n" BIP_MSG_PRE_ARG, pid ));

            if (pCollectionData->srcType == IpStreamerSrc_eIp)
            {
                NEXUS_Playpump_ClosePidChannel( pCollectionData->playpump, pCollectionData->pid[i].channel );
            }
            else
            {
                NEXUS_PidChannel_Close( pCollectionData->pid[i].channel );
            }
            pCollectionData->pid[i].channel = NULL;
            pCollectionData->pid[i].num     = 0;
            found = true;
            break;
        }
    }
    if (!found)
    {
        BDBG_ERR(( "failure closing pid channel:0x%04x - not found in list\n", pid ));
    }
} /* closePidChannel */

/* callback function called by si applib to collect si data */
static B_Error collectionFunc(
    B_PSIP_CollectionRequest *pRequest,
    void                     *context
    )
{
    NEXUS_PidChannelHandle pidChannel      = NULL;
    pPsiCollectionDataType pCollectionData = (pPsiCollectionDataType)context;
    NEXUS_MessageHandle    msg;

    BDBG_ASSERT( NULL != pRequest );
    BDBG_ASSERT( NULL != context );
    msg = (NEXUS_MessageHandle)pRequest->filterHandle;
    if (NULL == msg)
    {
        BDBG_ERR(( "invalid filterHandle received from SI applib!\n" ));
        return( B_ERROR_INVALID_PARAMETER );
    }

    switch (pRequest->cmd)
    {
        const uint8_t *buffer;
        size_t         size;

        case B_PSIP_eCollectStart:
        {
            BDBG_MSG((BIP_MSG_PRE_FMT "-=-=- B_PSIP_eCollectStart -=-=-\n" BIP_MSG_PRE_ARG));

            /*
             * Save off pRequest->callback and pRequest->callbackParam.
             * these need to be called when DataReady() is called.  this will
             * notify the si lib that it can begin processing the received data.
             * Si applib only allows us to call one API at a time (per filterHandle),
             * so we only have to keep track of the latest siRequest.callback
             * and siRequest.callbackParam (for our one and only filterHandle).
             */
            g_siRequest.callback      = pRequest->callback;
            g_siRequest.callbackParam = pRequest->callbackParam;

            pidChannel = openPidChannel( pCollectionData, pRequest->pid );
            startMessageFilter( pCollectionData, pRequest, pidChannel );
            break;
        }

        case B_PSIP_eCollectStop:
        {
            BDBG_MSG((BIP_MSG_PRE_FMT "-=-=- B_PSIP_eCollectStop -=-=-\n" BIP_MSG_PRE_ARG));
            stopMessageFilter( pCollectionData, pRequest );
            closePidChannel( pCollectionData, pRequest->pid );
            break;
        }

        case B_PSIP_eCollectGetBuffer:
        {
            BDBG_MSG((BIP_MSG_PRE_FMT "-=-=- B_PSIP_eCollectGetBuffer -=-=-\n" BIP_MSG_PRE_ARG));
            /* fall through for now... */
        }
        case B_PSIP_eCollectCopyBuffer:
        {
            /*printf("-=-=- B_PSIP_eCollectCopyBuffer -=-=-\n");*/
            if (NEXUS_SUCCESS == NEXUS_Message_GetBuffer( msg, (const void **)&buffer, &size ))
            {
                if (0 < size)
                {
                    BDBG_MSG((BIP_MSG_PRE_FMT "NEXUS_Message_GetBuffer() succeeded! size:%d\n" BIP_MSG_PRE_ARG, size ));
                    memcpy( pRequest->pBuffer, buffer, *( pRequest->pBufferLength )); /* copy valid data to request buffer */
                    *( pRequest->pBufferLength ) = BIP_MIN( *( pRequest->pBufferLength ), size );
                    NEXUS_Message_ReadComplete( msg, size );
                }
                else
                {
                    /* NEXUS_Message_GetBuffer can return size==0 (this is normal
                     * operation).  We will simply wait for the next data ready
                     * notification by returning a B_ERROR_RETRY to the PSIP applib
                     */
                    NEXUS_Message_ReadComplete( msg, size );
                    return( B_ERROR_PSIP_RETRY );
                }
            }
            else
            {
                BDBG_ERR((BIP_MSG_PRE_FMT "NEXUS_Message_GetBuffer() failed\n" BIP_MSG_PRE_ARG));

                return( B_ERROR_UNKNOWN );
            }
            break;
        }

        default:
        {
            BDBG_ERR(( "-=-=- invalid Command received:%d -=-=-\n", pRequest->cmd ));
            return( B_ERROR_INVALID_PARAMETER );

            break;
        }
    } /* switch */

    return( B_ERROR_SUCCESS );
} /* collectionFunc */

static void populateMediaInfoTrackFromTSPmtStruct(
    TS_PMT_stream *pStream,
    BIP_MediaInfoTrack   *pMediaInfoTrack
    )
{
    pMediaInfoTrack->trackId = pStream->elementary_PID;
    switch (pStream->stream_type)
    {
        case TS_PSI_ST_13818_2_Video:
        case TS_PSI_ST_ATSC_Video:
        {
            pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eVideo;
            pMediaInfoTrack->info.video.codec = NEXUS_VideoCodec_eMpeg2;
            break;
        }
        case TS_PSI_ST_11172_2_Video:
        {
            pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eVideo;
            pMediaInfoTrack->info.video.codec = NEXUS_VideoCodec_eMpeg1;
            break;
        }
        case TS_PSI_ST_14496_10_Video:
        {
            pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eVideo;
            pMediaInfoTrack->info.video.codec = NEXUS_VideoCodec_eH264;
            break;
        }
        case 0x27:
        case 0x24:
        {
            pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eVideo;
            pMediaInfoTrack->info.video.codec = NEXUS_VideoCodec_eH265;
            break;
        }
        case TS_PSI_ST_11172_3_Audio:
        case TS_PSI_ST_13818_3_Audio:
        {
            pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eAudio;
            pMediaInfoTrack->info.audio.codec = NEXUS_AudioCodec_eMpeg;
            break;
        }
        case TS_PSI_ST_ATSC_AC3:
        {
            pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eAudio;
            pMediaInfoTrack->info.audio.codec = NEXUS_AudioCodec_eAc3;
            break;
        }
        case TS_PSI_ST_ATSC_EAC3:
        {
            pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eAudio;
            pMediaInfoTrack->info.audio.codec = NEXUS_AudioCodec_eAc3Plus;
            break;
        }
        case TS_PSI_ST_13818_7_AAC:
        {
            pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eAudio;
            pMediaInfoTrack->info.audio.codec = NEXUS_AudioCodec_eAac;
            break;
        }
        case TS_PSI_ST_14496_3_Audio:
        {
            pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eAudio;
            pMediaInfoTrack->info.audio.codec = NEXUS_AudioCodec_eAacPlus;
            break;
        }
        default:
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "###### TODO: Unknown stream type: %x #####" BIP_MSG_PRE_ARG, pStream->stream_type ));
            pMediaInfoTrack->trackType             = BIP_MediaInfoTrackType_eOther;
            break;
        }
    } /* switch */
}

/*****************************************************************************************
  * Create BIPM_ediaInfoTrack from TS_PMT_Buffer received from TS_PMT_get* functions.
  ****************************************************************************************/
static BIP_Status createMediaInfoTrackFromTSPmtBuff(
    BIP_MediaInfoStream  *pMediaInfoStream,
    uint8_t                 *pBufPMT,/* In:pmt buff with pmt information.*/
    uint32_t             bufPMTLength,/* In:pmt buffer length.*/
    TS_PAT_program       *pTsPatProgram
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    unsigned trackId;
    BIP_MediaInfoTrack   *pMediaInfoTrack =  NULL;
    int j;
    TS_PMT_stream stream;
#ifndef STREAMER_CABLECARD_SUPPORT
    EPID epid;
#endif

    BDBG_ASSERT( NULL != pMediaInfoStream );

    /************************ Collect pcr track *********************/
    trackId = TS_PMT_getPcrPid( pBufPMT, bufPMTLength);
    if (trackId > 1)
    {
        /*Create BIP_MediaInfoTrack for pcr track */
        pMediaInfoTrack = B_Os_Calloc(1,sizeof(BIP_MediaInfoTrack));
        BIP_CHECK_GOTO(( pMediaInfoTrack!=NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfoTrack)  ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        /* Populate pcr track related information.*/
        pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_ePcr;
        pMediaInfoTrack->trackId = trackId;
    }
    /* Adding track to track list. */
    BIP_MEDIAINFO_TRACKLIST_INSERT(&pMediaInfoStream->pFirstTrackInfoForStream, pMediaInfoTrack);
    /* Update numTracks in MediaInfoStream. */
    pMediaInfoStream->numberOfTracks++;
    /* Track added to track list.*/

    rc = addMediaInfoTrackToTrackGroup(pMediaInfoStream, pMediaInfoTrack, pTsPatProgram->program_number);
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addMediaInfoTrackToTrackGroup failed" ), error, rc, rc );

    /************************ Pcr track Collected *********************/

    /**************** Now collect the other tracks.*********************/
    for (j = 0; j < TS_PMT_getNumStreams( pBufPMT, bufPMTLength ); j++)  /* this give you the track */
    {
        B_Os_Memset( &stream, 0, sizeof( stream ));
        TS_PMT_getStream( pBufPMT, bufPMTLength, j, &stream );
        BDBG_MSG(( BIP_MSG_PRE_FMT "j %d, stream_type:0x%02X, PID:0x%04X\n" BIP_MSG_PRE_ARG, j, stream.stream_type, stream.elementary_PID ));

        /*Create BIP_MediaInfoTrack for pcr track */
        pMediaInfoTrack = NULL;
        pMediaInfoTrack = B_Os_Calloc(1,sizeof(BIP_MediaInfoTrack));
        BIP_CHECK_GOTO(( pMediaInfoTrack!=NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfoTrack)  ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        populateMediaInfoTrackFromTSPmtStruct( &stream, pMediaInfoTrack );

        /* Adding track to track list. */
        BIP_MEDIAINFO_TRACKLIST_INSERT(&pMediaInfoStream->pFirstTrackInfoForStream, pMediaInfoTrack);
        /* Update numTracks in MediaInfoStream. */
        pMediaInfoStream->numberOfTracks++;
        /* Track added to track list.*/

        rc = addMediaInfoTrackToTrackGroup(pMediaInfoStream, pMediaInfoTrack, pTsPatProgram->program_number);
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addMediaInfoTrackToTrackGroup failed" ), error, rc, rc );

#ifndef STREAMER_CABLECARD_SUPPORT
        /*TODO: Later check whether we at all need to keep the foloowing four lines of code.*/
        B_Os_Memset( &epid, 0, sizeof( epid ));
        tsPsi_procStreamDescriptors( pBufPMT, bufPMTLength, j, &epid );
        if (epid.ca_pid)
        {
            BDBG_MSG((BIP_MSG_PRE_FMT "even program 0x%x has ca pid 0x%x, we are not ignoring it" BIP_MSG_PRE_ARG, pTsPatProgram->PID, epid.ca_pid ));
        }
#endif
    }
    /**************** All other tracks collected *********************/

error:
    return rc;
}

#define PMT_BUF_LENGTH 4096
#define PAT_BUF_LENGTH 4096
/********************************************************
* Probe tuner data to get media info meta data.
*********************************************************/
BIP_Status BIP_MediaInfo_ProbeTunerForMediaInfo(
                                    BIP_MediaInfoHandle hMediaInfo,
                                    NEXUS_ParserBand parserBand
                                    )
{
    BIP_Status rc    = BIP_SUCCESS;
    psiCollectionDataType collectionData;
    B_Error               errCode;
    NEXUS_MessageSettings settings;
    NEXUS_MessageHandle   msg = NULL;
    B_PSIP_Handle         si  = NULL;
    B_PSIP_Settings       si_settings;
    B_PSIP_ApiSettings    settingsApi;
    TS_PAT_program        program;
    uint8_t               i;
    uint8_t              *pBufPAT         = NULL;
    uint32_t             bufPATLength = PAT_BUF_LENGTH;
    uint8_t              *pBufPMT         = NULL;
    uint32_t             bufPMTLength = PMT_BUF_LENGTH;

    B_EventHandle         dataReadyEvent = NULL;
    bool                  nitFound = 0;
    BIP_MediaInfoStream  *pMediaInfoStream;
    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings;

#ifndef STREAMER_CABLECARD_SUPPORT
    PROGRAM_INFO_T programInfo;
    B_Os_Memset( &programInfo, 0, sizeof( programInfo ));
#endif

    BDBG_ENTER(BIP_MediaInfo_ProbeTunerForMediaInfo);

    BDBG_ASSERT(hMediaInfo);
    BDBG_OBJECT_ASSERT(hMediaInfo,BIP_MediaInfo);

    /* Initialize mediaInfoStream from MediaInfo object.*/
    pMediaInfoStream = &(hMediaInfo->mediaInfoStream);
    pMediaInfoCreateSettings = &(hMediaInfo->createSettings);

    /* Populate MediaInfoStream structure */
    /* TODO: Check whether this has any issue , for Live case I always set it as bstream_mpeg_type_ts. */
    hMediaInfo->mediaInfoStream.transportType = BIP_MediaInfo_ConvertBmediaMpegTypeToNexus( bstream_mpeg_type_ts);
    hMediaInfo->mediaInfoStream.liveChannel = true;

    /* Allocate pBufPAT and pBufPMT. */
    pBufPAT = B_Os_Calloc( 1, bufPATLength);
    BIP_CHECK_GOTO(( pBufPAT != NULL ), ( "Failed to allocate memory (%d bytes) for pBufPAT ", PAT_BUF_LENGTH ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    pBufPMT = B_Os_Calloc( 1, bufPMTLength);
    BIP_CHECK_GOTO(( pBufPMT != NULL ), ( "Failed to allocate memory (%d bytes) for pBufPMT ", PMT_BUF_LENGTH ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Start stream  */
    B_Os_Memset( &collectionData, 0, sizeof( psiCollectionDataType ));
    /* collectionFunc works same way for all srcType other than for type IpStreamerSrc_eIp.
    Now BIP_MediaInfo_ProbeTunerForMediaInfo only works for sat and qam input.so collectionData.srcType is set to one of them.
    Later based on requirements we can add srcType in BIP_MediaInfoCreateSettings. */
    collectionData.srcType    = IpStreamerSrc_eSat;
    collectionData.live       = true;
    collectionData.parserBand = parserBand;

    NEXUS_Message_GetDefaultSettings( &settings );
    settings.dataReady.callback = DataReady;
    settings.dataReady.context  = NULL;
    msg = NEXUS_Message_Open( &settings );
    BIP_CHECK_GOTO(( msg != NULL), ( "PSI - NEXUS_Message_Open failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    B_PSIP_GetDefaultSettings( &si_settings );
    si_settings.PatTimeout = 800;
    si_settings.PmtTimeout = 800;
    si = B_PSIP_Open( &si_settings, collectionFunc, &collectionData );
    BIP_CHECK_GOTO(( si ), ( "PSI - Unable to open SI applib" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Starting PSI gathering\n" BIP_MSG_PRE_ARG ));

    errCode = B_PSIP_Start( si );
    BIP_CHECK_GOTO(( errCode == B_ERROR_SUCCESS ), ( "PSI - Unable to start SI data collection, err %d\n", errCode ), error, BIP_ERR_INTERNAL, rc );

    dataReadyEvent = B_Event_Create( NULL );
    BIP_CHECK_GOTO(( dataReadyEvent != NULL ), ( "Event Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    B_PSIP_GetDefaultApiSettings( &settingsApi );
    settingsApi.siHandle               = si;
    settingsApi.filterHandle           = msg;
    settingsApi.dataReadyCallback      = SiDataReady;
    settingsApi.dataReadyCallbackParam = dataReadyEvent;
    settingsApi.timeout                = 65000;

    B_Event_Reset((B_EventHandle)settingsApi.dataReadyCallbackParam );
    errCode = B_PSIP_GetPAT( &settingsApi, pBufPAT, &bufPATLength );
    BIP_CHECK_GOTO(( errCode == B_ERROR_SUCCESS ), ( "B_PSIP_GetPAT() failed \n" ), error, errCode, rc );

    /* wait for async response from si - wait on dataReadyEvent */
    if (B_Event_Wait((B_EventHandle)settingsApi.dataReadyCallbackParam, pMediaInfoCreateSettings->psiAcquireTimeoutInMs ))
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "Failed to find PAT table ...\n" BIP_MSG_PRE_ARG));
        rc = BIP_INF_TIMEOUT;
        goto error;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Received response from SI, len = %d!\n" BIP_MSG_PRE_ARG, bufPATLength));

    if( bufPATLength > 0)
    {
        for (i = 0; i<TS_PAT_getNumPrograms( pBufPAT ) && ( TS_PAT_getProgram( pBufPAT, bufPATLength, i, &program )==BERR_SUCCESS ); i++)
        {
            if (program.program_number == 0)
            {
                nitFound = 1;
                /* Right now we don't use nit information, so continue */
                continue;
            }
            B_Event_Reset((B_EventHandle)settingsApi.dataReadyCallbackParam);
            /* Reset pBufPMT and buPMTLength. */
            bufPMTLength = PMT_BUF_LENGTH;
            B_Os_Memset( pBufPMT, 0, bufPMTLength );
            if (B_ERROR_SUCCESS != B_PSIP_GetPMT( &settingsApi, program.PID, pBufPMT, &bufPMTLength ))
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "B_PSIP_GetPMT() failed\n" BIP_MSG_PRE_FMT));
                continue;
            }
            /* wait for async response from si - wait on dataReadyEvent */
            if (B_Event_Wait((B_EventHandle)settingsApi.dataReadyCallbackParam,  pMediaInfoCreateSettings->psiAcquireTimeoutInMs ))
            {
                BDBG_ERR((BIP_MSG_PRE_FMT "Failed to find PMT table ...\n" BIP_MSG_PRE_FMT));
                rc = BIP_INF_TIMEOUT;
                goto error;
            }

            /* On successful aquisition of Pmt add it to our TrackGroupList irrespective of whether it has any streams in it or not.*/
            rc = addPmtPidToMediaInfoTrackGroup(
                        pMediaInfoStream,
                        program.program_number,
                        program.PID
                        );
            BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "addPmtPidToMediaInfoTrackGroup failed \n" ), error, rc, rc );

            BDBG_MSG(( BIP_MSG_PRE_FMT "Received PMT: size:%d. Number of streams(tracks for bip) in this program %d\n" BIP_MSG_PRE_ARG, bufPMTLength, TS_PMT_getNumStreams( pBufPMT, bufPMTLength )));

            if(TS_PMT_getNumStreams( pBufPMT, bufPMTLength ) == 0)
            {
                BDBG_WRN((BIP_MSG_PRE_FMT "Found zero  tracks in this program \n" BIP_MSG_PRE_FMT));
                continue;
            }
#ifndef STREAMER_CABLECARD_SUPPORT
            /* find and process Program descriptors */
            tsPsi_procProgDescriptors( pBufPMT, bufPMTLength, &programInfo );
            if (programInfo.ca_pid)
            {
                /* we are no longer ignoring the programs w/ ca_pid set as it was causing us to skip over even the clear channels */
                /* looks like this descriptor is set even for clear streams */
                BDBG_MSG(( BIP_MSG_PRE_FMT "Even though Program # %d, pid 0x%x seems encrypted, ca_pid 0x%x, we are not skipping it" BIP_MSG_PRE_ARG, program.program_number, program.PID, programInfo.ca_pid ));
            }
#endif /* ifndef STREAMER_CABLECARD_SUPPORT */

            if ( bufPMTLength > 0)
            {
                rc = createMediaInfoTrackFromTSPmtBuff( pMediaInfoStream, pBufPMT,bufPMTLength,&program );
                BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "createMediaInfoTrackFromTSPmtBuff failed\\n" ), error, rc, rc );
            }

        }/* end of for loop */
    }

error:
    if(pBufPAT)
    {
        B_Os_Free(pBufPAT);
    }
    if(pBufPMT)
    {
        B_Os_Free(pBufPMT);
    }
    if (si)
    {
        B_PSIP_Close( si );
    }
    if (msg)
    {
        NEXUS_Message_Close( msg );
    }
    if (dataReadyEvent)
    {
        B_Event_Destroy( dataReadyEvent );
    }

    BDBG_LEAVE( BIP_MediaInfo_ProbeTunerForMediaInfo );
    return rc;
}

/********************************************************
* Create a BIP_MediaInfo object from a NEXUS_ParserBand.
*********************************************************/
BIP_MediaInfoHandle BIP_MediaInfo_CreateFromParserBand(
    NEXUS_ParserBand parserBand,                     /*!< parserBand connected to the media source. */
                                                     /*!< If NEXUS_ParserBand_eInvalid , the MediaInfo will be read from an existing media Info file specified by pInfoFileAbsolutePathName. */

    const char      *pInfoFileAbsolutePathName,      /*!< Absolute path name to the Info file. If info file doesn't exist then the MediaInfo will be acquired from the parserBand and stored in the Info file. */
                                                     /*!< If NULL, the MediaInfo will be acquired from the parserBand. */

    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings   /*!< Optional address of a BIP_MediaInfoCreateSettings structure.  */
                                                            /*!< Passing NULL will use default settings.*/
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_MediaInfoHandle hMediaInfo = NULL;
    BIP_MediaInfoCreateSettings defaultMediaInfoCreateSettings;

    BDBG_ENTER( BIP_MediaInfo_CreateFromParserBand );

    if(NULL == pMediaInfoCreateSettings)
    {
        BIP_MediaInfo_GetDefaultCreateSettings(&defaultMediaInfoCreateSettings);
        pMediaInfoCreateSettings = &defaultMediaInfoCreateSettings;
    }

    hMediaInfo = B_Os_Calloc( 1, sizeof( BIP_MediaInfo ));
    BIP_CHECK_GOTO(( hMediaInfo != NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfo) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( hMediaInfo, BIP_MediaInfo );

    hMediaInfo->createSettings = *pMediaInfoCreateSettings;

    if(parserBand == NEXUS_ParserBand_eInvalid)
    {
        /* In this case it will only work if pInfoFileAbsolutePathName points to a valid info file for tuner input.*/
        if(pInfoFileAbsolutePathName == NULL)
        {
            rc = BIP_ERR_INVALID_PARAMETER;
            BDBG_ERR((BIP_MSG_PRE_FMT "Invalid parameter Can't create BIP_MediaInfo object." BIP_MSG_PRE_ARG));
            goto error;
        }
        else
        {
            rc = BIP_MediaInfo_CheckForValidInfoFile(pInfoFileAbsolutePathName);
            BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_CheckForValidInfoFile failed" ), error, rc, rc );

            rc = BIP_MediaInfo_RetriveMetaDataFromInfoFile(
                     pInfoFileAbsolutePathName,
                     hMediaInfo
                     );
            BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_RetriveMetaDataFromInfoFile failed" ), error, rc, rc );
        }
    }
    else    /* valid parserBand */
    {
        /* Check whether infoFile exist.*/
        if(NULL != pInfoFileAbsolutePathName)
        {
            rc = BIP_MediaInfo_CheckForValidInfoFile(pInfoFileAbsolutePathName);
            if((BIP_SUCCESS != rc) || (true == hMediaInfo->createSettings.reAcquireInfo))
            {
                /* Reacquire the media info meta data from media file. So we internally set the reAcquire flag irrespective of whether it is set by app
                   in case when BIP_MediaInfo_CheckForValidInfoFile failed(since in bothe cases behaviour is same).On the basis of this flag we will save the new media info file.*/
                hMediaInfo->createSettings.reAcquireInfo = true;

                BDBG_MSG(( BIP_MSG_PRE_FMT "Calling BIP_MediaInfo_ProbeTunerForMediaInfo to regenerate info file " BIP_MSG_PRE_ARG));

                rc = BIP_MediaInfo_ProbeTunerForMediaInfo(
                                    hMediaInfo,
                                    parserBand
                                    );
                BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_ProbeTunerForMediaInfo failed" ), error, rc, rc );
            }
            else
            {
                /* Valid media info file exist and reAcquire flag is not set so retrive the meta data from existing info file. */
                BDBG_MSG(( BIP_MSG_PRE_FMT "Calling BIP_MediaInfo_RetriveMetaDataFromInfoFile to retrive medaiInfo meta data from existing info file " BIP_MSG_PRE_ARG));
                rc = BIP_MediaInfo_RetriveMetaDataFromInfoFile(
                         pInfoFileAbsolutePathName,
                         hMediaInfo
                         );
                BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_RetriveMetaDataFromInfoFile failed" ), error, rc, rc );
            }

        }
        else /*(NULL == pInfoFileAbsolutePathName) */
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "Calling BIP_MediaInfo_ProbeTunerForMediaInfo to regenerate info file " BIP_MSG_PRE_ARG));
            rc = BIP_MediaInfo_ProbeTunerForMediaInfo(
                                    hMediaInfo,
                                    parserBand
                                    );
            BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_ProbeTunerForMediaInfo failed" ), error, rc, rc );
        }

    }

    if(hMediaInfo->createSettings.reAcquireInfo)
    {
        /* Create xml tree to generate the new xml info file.*/
        rc = BIP_MediaInfo_CreateXmlTree(hMediaInfo, pInfoFileAbsolutePathName);
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_CreateXmlTree failed" ), error, rc, rc );
    }

    BDBG_LEAVE( BIP_MediaInfo_CreateFromParserBand );
    return hMediaInfo;
error:
    BIP_MediaInfo_Destroy(hMediaInfo);
    BDBG_LEAVE( BIP_MediaInfo_CreateFromParserBand );
    return NULL;
}

static BIP_Status createMediaInfoTrackForMainVideoFromPbipPsi(
    const B_PlaybackIpPsiInfo *pPsi,
    BIP_MediaInfoStream  *pMediaInfoStream
    )
{
   BIP_Status rc    = BIP_SUCCESS;
   BIP_MediaInfoTrack   *pMediaInfoTrack =  NULL;

   BDBG_ASSERT( NULL != pMediaInfoStream );
   BDBG_ASSERT( NULL != pPsi );

   pMediaInfoTrack = B_Os_Calloc(1,sizeof(BIP_MediaInfoTrack));
   BIP_CHECK_GOTO(( pMediaInfoTrack!=NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfoTrack)  ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

   /* Now populate MainVideo track from PbipPsi structure. */
   pMediaInfoTrack->trackId = pPsi->videoPid;
   pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eVideo;
   pMediaInfoTrack->info.video.bitrate = pPsi->videoBitrate;
   pMediaInfoTrack->info.video.codec = pPsi->videoCodec;
   pMediaInfoTrack->info.video.colorDepth = pPsi->colorDepth;
   /*pMediaInfoTrack->info.video.frameRate = pPsi->; TODO: Need to fill it later */
   pMediaInfoTrack->info.video.height = pPsi->videoHeight;
   pMediaInfoTrack->info.video.width = pPsi->videoWidth;

   /* Adding track to track list. */
   BIP_MEDIAINFO_TRACKLIST_INSERT(&pMediaInfoStream->pFirstTrackInfoForStream, pMediaInfoTrack);
   /* Track added to track list.*/

   pMediaInfoStream->numberOfTracks++;
error:
   return rc;
}

static char *addLanguageInIso6392Format(
    const char *pLanguageRecieved
    )
{
    char *pLanguage = NULL;

    /* Extract language , if it is in ISO639-1 then convert it to Iso639-2 format.*/
    if(strlen(pLanguageRecieved) == 2)
    {
       const char *pCode6392 = NULL;
       pCode6392 = BIP_MediaInfo_GetIso6392CodeFrom6391Code(pLanguageRecieved);

       if(pCode6392 == NULL)
       {
           BDBG_WRN((BIP_MSG_PRE_FMT " Language code |%s| not supported. Check BIP_MediaInfoLanguageCode table." BIP_MSG_PRE_ARG, pLanguageRecieved));
       }
       else
       {
           pLanguage = B_Os_Calloc(1,BIP_MEDIA_INFO_LANGUAGE_FIELD_SIZE);
           memcpy( pLanguage, pCode6392, (strlen(pCode6392)+1)); /* copy valid data to request buffer */
       }
    }
    else /* Directly copy assuming it is ISO639-2 code.*/
    {
       pLanguage = B_Os_Calloc(1,BIP_MEDIA_INFO_LANGUAGE_FIELD_SIZE);
       memcpy( pLanguage, pLanguageRecieved, (strlen(pLanguageRecieved)+1)); /* copy valid data to request buffer */
    }
    return (pLanguage);
}

static BIP_Status createMediaInfoTrackForMainAudioFromPbipPsi(
    const B_PlaybackIpPsiInfo *pPsi,
    BIP_MediaInfoStream  *pMediaInfoStream
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_MediaInfoTrack   *pMediaInfoTrack =  NULL;

    BDBG_ASSERT( NULL != pMediaInfoStream );
    BDBG_ASSERT( NULL != pPsi );

    pMediaInfoTrack = B_Os_Calloc(1,sizeof(BIP_MediaInfoTrack));
    BIP_CHECK_GOTO(( pMediaInfoTrack!=NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfoTrack)  ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Now populate MainAudio track from PbipPsi structure. */
    pMediaInfoTrack->trackId = pPsi->audioPid;
    pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eAudio;

    pMediaInfoTrack->info.audio.bitrate = pPsi->audioBitrate;
    pMediaInfoTrack->info.audio.channelCount = pPsi->audioNumChannels;
    pMediaInfoTrack->info.audio.codec = pPsi->audioCodec;
    pMediaInfoTrack->info.audio.sampleRate = pPsi->audioSampleRate;
    pMediaInfoTrack->info.audio.sampleSize = pPsi->audioBitsPerSample;

    /* Extract language , if it is in ISO639-1 then convert it to Iso639-2 format.*/
    if(pPsi->mainAudioLanguage)
    {
         pMediaInfoTrack->info.audio.pLanguage = addLanguageInIso6392Format( pPsi->mainAudioLanguage);
    }

    /* Adding track to track list. */
    BIP_MEDIAINFO_TRACKLIST_INSERT(&pMediaInfoStream->pFirstTrackInfoForStream, pMediaInfoTrack);
    /* Track added to track list.*/

    pMediaInfoStream->numberOfTracks++;
error:
    return rc;
}

static BIP_Status createMediaInfoTrackForPcrFromPbipPsi(
    const B_PlaybackIpPsiInfo *pPsi,
    BIP_MediaInfoStream  *pMediaInfoStream
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_MediaInfoTrack   *pMediaInfoTrack =  NULL;

    BDBG_ASSERT( NULL != pMediaInfoStream );
    BDBG_ASSERT( NULL != pPsi );

    pMediaInfoTrack = B_Os_Calloc(1,sizeof(BIP_MediaInfoTrack));
    BIP_CHECK_GOTO(( pMediaInfoTrack!=NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfoTrack)  ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Now populate MainAudio track from PbipPsi structure. */
    pMediaInfoTrack->trackId = pPsi->pcrPid;
    pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_ePcr;

     /* Adding track to track list. */
    BIP_MEDIAINFO_TRACKLIST_INSERT(&pMediaInfoStream->pFirstTrackInfoForStream, pMediaInfoTrack);
    /* Track added to track list.*/

    pMediaInfoStream->numberOfTracks++;
error:
    return rc;
}

#define BIP_MEDIA_INFO_VIRTUAL_TRACK_ID_OFFSET 1

bool checkWhetherTrackIdAlreadyExist(
    BIP_MediaInfoStream  *pMediaInfoStream,
    unsigned trackId
    )
{
    BIP_MediaInfoTrack   *pMediaInfoTrack =  NULL;

    for(pMediaInfoTrack = pMediaInfoStream->pFirstTrackInfoForStream;
        pMediaInfoTrack;
        pMediaInfoTrack = pMediaInfoTrack->pNextTrackForStream
       )
    {
        if(pMediaInfoTrack->trackId == trackId)
        {
            return true;
        }
    }
    return false;
}

unsigned getValidVirtualTrackId(
    BIP_MediaInfoStream         *pMediaInfoStream,
    unsigned                     lastTrackId
    )
{
    unsigned virtualTrackId;
    /* first determine the first virtual trackId. */
    virtualTrackId = lastTrackId + BIP_MEDIA_INFO_VIRTUAL_TRACK_ID_OFFSET;

    while(checkWhetherTrackIdAlreadyExist(pMediaInfoStream, virtualTrackId))
    {
        virtualTrackId+=BIP_MEDIA_INFO_VIRTUAL_TRACK_ID_OFFSET;
    }
    return (virtualTrackId);
}

static BIP_Status createMediaInfoTracksForExtraHlsAudioTracks(
    const B_PlaybackIpPsiInfo   *pPsi,
    BIP_MediaInfoStream         *pMediaInfoStream,
    unsigned                     lastTrackId
    )
{
    BIP_Status rc     = BIP_SUCCESS;
    unsigned   virtualTrackId;
    unsigned i;

    BDBG_ASSERT( NULL != pMediaInfoStream );
    BDBG_ASSERT( NULL != pPsi );

    for (i=0; i < pPsi->hlsAltAudioRenditionCount ; i ++)
    {
        BIP_MediaInfoTrack   *pMediaInfoTrack =  NULL;

        pMediaInfoTrack = B_Os_Calloc(1,sizeof(BIP_MediaInfoTrack));
        BIP_CHECK_GOTO(( pMediaInfoTrack!=NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfoTrack)  ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        /* Get virtualTrackId*/
        virtualTrackId = getValidVirtualTrackId( pMediaInfoStream, lastTrackId );
        pMediaInfoTrack->trackId = virtualTrackId;
        lastTrackId = virtualTrackId;

        pMediaInfoTrack->trackType = BIP_MediaInfoTrackType_eAudio;
        pMediaInfoTrack->info.audio.codec = pPsi->hlsAltAudioRenditionInfo[i].codec;
        pMediaInfoTrack->info.audio.hlsSessionEnabled = true;

        if(pPsi->hlsAltAudioRenditionInfo[i].language)
        {
            pMediaInfoTrack->info.audio.pLanguage = addLanguageInIso6392Format(pPsi->hlsAltAudioRenditionInfo[i].language);
        }

        /* Add hls specifc info. */
        {
            /* TODO: "defaultAudio" If Sanjeev confirms that hls player dpoesn't need this then we can remove it later.*/
            pMediaInfoTrack->info.audio.containerSpecificInfo.hls.defaultAudio = pPsi->hlsAltAudioRenditionInfo[i].defaultAudio;
            pMediaInfoTrack->info.audio.containerSpecificInfo.hls.hlsAudioPid = pPsi->hlsAltAudioRenditionInfo[i].pid;

            /* this preserves the original language code*/
            if(pPsi->hlsAltAudioRenditionInfo[i].language)
            {
                pMediaInfoTrack->info.audio.containerSpecificInfo.hls.pHlsLanguageCode = B_Os_Calloc(1,(strlen(pPsi->hlsAltAudioRenditionInfo[i].language)+1));
                memcpy(
                    pMediaInfoTrack->info.audio.containerSpecificInfo.hls.pHlsLanguageCode,
                    pPsi->hlsAltAudioRenditionInfo[i].language,
                    (strlen(pPsi->hlsAltAudioRenditionInfo[i].language)+1)
                    );
            }

            if(pPsi->hlsAltAudioRenditionInfo[i].languageName)
            {
                pMediaInfoTrack->info.audio.containerSpecificInfo.hls.pLanguageName = B_Os_Calloc(1,(strlen(pPsi->hlsAltAudioRenditionInfo[i].languageName) + 1));
                memcpy(
                    pMediaInfoTrack->info.audio.containerSpecificInfo.hls.pLanguageName,
                    pPsi->hlsAltAudioRenditionInfo[i].languageName,
                    (strlen(pPsi->hlsAltAudioRenditionInfo[i].languageName)+1)
                    ); /* copy valid data to request buffer */
            }
            pMediaInfoTrack->info.audio.containerSpecificInfo.hls.requiresSecondPlaypumForAudio = pPsi->hlsAltAudioRenditionInfo[i].requiresPlaypump2;

            if(pPsi->hlsAltAudioRenditionInfo[i].groupId)
            {
                pMediaInfoTrack->info.audio.containerSpecificInfo.hls.pGroupId = B_Os_Calloc(1,(strlen(pPsi->hlsAltAudioRenditionInfo[i].groupId) + 1));
                memcpy(
                    pMediaInfoTrack->info.audio.containerSpecificInfo.hls.pGroupId,
                    pPsi->hlsAltAudioRenditionInfo[i].groupId,
                    (strlen(pPsi->hlsAltAudioRenditionInfo[i].groupId) + 1)
                    );
            }
            /* TODO: "hlsExtraAudioSpecificContainerType" If Sanjeev confirms that hls player dpoesn't need this then we can remove it later.*/
            pMediaInfoTrack->info.audio.containerSpecificInfo.hls.hlsExtraAudioSpecificContainerType = pPsi->hlsAltAudioRenditionInfo[i].containerType;
        }/* hls specific informations are added.*/

        /* Now track is ready to be added to the track list */
        /* Adding track to track list. */
        BIP_MEDIAINFO_TRACKLIST_INSERT(&pMediaInfoStream->pFirstTrackInfoForStream, pMediaInfoTrack);
    }
error:
    return rc;

}

static BIP_Status BIP_MediaInfo_GenerateBipMediaInfoFromPbipPsi(
     const B_PlaybackIpPsiInfo *pPsi,
     BIP_MediaInfoHandle hMediaInfo
     )
{
     BIP_Status rc    = BIP_SUCCESS;
     int lastTrackId  = 0;

     BDBG_ASSERT( NULL != hMediaInfo );
     BDBG_ASSERT( NULL != pPsi );

     if(pPsi == NULL)
     {
        rc = BIP_ERR_INVALID_PARAMETER;
        BDBG_ERR((BIP_MSG_PRE_FMT "Invalid parameter." BIP_MSG_PRE_ARG));
        goto error;
     }

     /* Set mediaInfoStream specific information. */
     hMediaInfo->mediaInfoStream.avgBitRate = pPsi->avgBitRate;
     hMediaInfo->mediaInfoStream.contentLength = pPsi->contentLength;
     hMediaInfo->mediaInfoStream.durationInMs = pPsi->duration;
     hMediaInfo->mediaInfoStream.liveChannel = pPsi->liveChannel;
     hMediaInfo->mediaInfoStream.transportTimeStampEnabled = pPsi->transportTimeStampEnabled;
     hMediaInfo->mediaInfoStream.transportType = pPsi->mpegType;

     if(pPsi->pcrPid != 0)
     {
         rc = createMediaInfoTrackForPcrFromPbipPsi(pPsi, &hMediaInfo->mediaInfoStream );
         BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "crearteMediaInfoTrackForPcrFromPbipPsi failed\\n" ), error, rc, rc );
         /* Update lastTrackId */
         lastTrackId = pPsi->pcrPid;
     }
     if(pPsi->videoPid != 0)
     {
         rc = createMediaInfoTrackForMainVideoFromPbipPsi(pPsi, &hMediaInfo->mediaInfoStream );
         BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "crearteMediaInfoTrackForMainVideoFromPbipPsi failed\\n" ), error, rc, rc );
         /* Update lastTrackId */
         lastTrackId = pPsi->videoPid;
     }

     if(pPsi->audioPid != 0)
     {
         rc = createMediaInfoTrackForMainAudioFromPbipPsi(pPsi, &hMediaInfo->mediaInfoStream);
         BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "crearteMediaInfoTrackForMainAudioFromPbipPsi failed\\n" ), error, rc, rc );

         /* Update lastTrackId */
         /* usually audio track is the last track and assuming that we collect audioTrackId last */
         lastTrackId = pPsi->audioPid;
     }

     if(pPsi->hlsSessionEnabled)
     {
         rc = createMediaInfoTracksForExtraHlsAudioTracks(pPsi, &hMediaInfo->mediaInfoStream, lastTrackId);
         BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "createMediaInfoTracksForExtraHlsAudioTracks failed\\n" ), error, rc, rc );
     }

error:
    return rc;
}


/**
Summary:
Create a BIP_MediaInfo object from a B_PlaybackIpPsiInfo.
**/
BIP_MediaInfoHandle BIP_MediaInfo_CreateFromPbipPsi_priv(
    const B_PlaybackIpPsiInfo *pPsi,
    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings   /*!< Optional address of a BIP_MediaInfoCreateSettings structure.  */
                                                            /*!< Passing NULL will use default settings.*/
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_MediaInfoHandle hMediaInfo = NULL;
    BIP_MediaInfoCreateSettings defaultMediaInfoCreateSettings;

    BDBG_ENTER( BIP_MediaInfo_CreateFromPbipPsi_priv );
    BDBG_ASSERT(pPsi);

    if(NULL == pMediaInfoCreateSettings)
    {
        BIP_MediaInfo_GetDefaultCreateSettings(&defaultMediaInfoCreateSettings);
        pMediaInfoCreateSettings = &defaultMediaInfoCreateSettings;
    }

    hMediaInfo = B_Os_Calloc( 1, sizeof( BIP_MediaInfo ));
    BIP_CHECK_GOTO(( hMediaInfo != NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfo) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    BDBG_OBJECT_SET( hMediaInfo, BIP_MediaInfo );

    /* Create hAbsoluteMediaPath object, later it will be filled with data.*/
    hMediaInfo->hAbsoluteMediaPath = BIP_String_Create();
    BIP_CHECK_GOTO(( hMediaInfo->hAbsoluteMediaPath != NULL ), ( "Failed to allocate memory for BIP_String object"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hMediaInfo->createSettings = *pMediaInfoCreateSettings;

    if(pPsi == NULL)
    {
        rc = BIP_ERR_INVALID_PARAMETER;
        BDBG_ERR((BIP_MSG_PRE_FMT "Invalid parameter." BIP_MSG_PRE_ARG));
        goto error;
    }
    rc = BIP_MediaInfo_GenerateBipMediaInfoFromPbipPsi(pPsi, hMediaInfo );
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_GenerateBipMediaInfoFromPbipPsi failed" ), error, rc, rc );

    BDBG_LEAVE( BIP_MediaInfo_CreateFromPbipPsi_priv );
    return hMediaInfo;
error:
    if(hMediaInfo)
    {
        BIP_MediaInfo_Destroy(hMediaInfo);
    }
    BDBG_LEAVE( BIP_MediaInfo_CreateFromPbipPsi_priv );
    return NULL;
}

/**
Summary:
Create a BIP_MediaInfo object from a bmedia_probe_stream.
**/
BIP_MediaInfoHandle BIP_MediaInfo_CreateFromBMediaStream_priv(
    const bmedia_probe_stream   *pStream,
    unsigned                    durationInMs,
    int64_t                     contentLength,
    bool                        liveChannel,
    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings   /*!< Optional address of a BIP_MediaInfoCreateSettings structure.  */
                                                            /*!< Passing NULL will use default settings.*/
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_MediaInfoHandle hMediaInfo = NULL;
    BIP_MediaInfoCreateSettings defaultMediaInfoCreateSettings;

    BDBG_ENTER( BIP_MediaInfo_CreateFromBMediaStream_priv );
    BDBG_ASSERT(pStream);

    if(NULL == pMediaInfoCreateSettings)
    {
        BIP_MediaInfo_GetDefaultCreateSettings(&defaultMediaInfoCreateSettings);
        pMediaInfoCreateSettings = &defaultMediaInfoCreateSettings;
    }

    hMediaInfo = B_Os_Calloc( 1, sizeof( BIP_MediaInfo ));
    BIP_CHECK_GOTO(( hMediaInfo != NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfo) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( hMediaInfo, BIP_MediaInfo );

    /* Create hAbsoluteMediaPath object, later it will be filled with data.*/
    hMediaInfo->hAbsoluteMediaPath = BIP_String_Create();
    BIP_CHECK_GOTO(( hMediaInfo->hAbsoluteMediaPath != NULL ), ( "Failed to allocate memory for BIP_String object"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hMediaInfo->createSettings = *pMediaInfoCreateSettings;

    if(pStream == NULL)
    {
        rc = BIP_ERR_INVALID_PARAMETER;
        BDBG_ERR((BIP_MSG_PRE_FMT "Invalid parameter." BIP_MSG_PRE_ARG));
        goto error;
    }

    rc = BIP_MediaInfo_GenerateBipMediaInfoFromBMediaStream(pStream, hMediaInfo );
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_GenerateBipMediaInfoFromBMediaStream failed" ), error, rc, rc );

    /* Fill-in app provided stream parameters. */
    hMediaInfo->mediaInfoStream.contentLength = contentLength;
    hMediaInfo->mediaInfoStream.liveChannel = liveChannel;
    if (durationInMs) hMediaInfo->mediaInfoStream.durationInMs = durationInMs;

    BDBG_LEAVE( BIP_MediaInfo_CreateFromBMediaStream_priv );
    return hMediaInfo;
error:
    if(hMediaInfo)
    {
        BIP_MediaInfo_Destroy(hMediaInfo);
    }
    BDBG_LEAVE( BIP_MediaInfo_CreateFromBMediaStream_priv );
    return NULL;
}

/* Create media info object for a media file.*/
BIP_MediaInfoHandle BIP_MediaInfo_CreateFromMediaFile(
    const char *pMediaFileAbsolutePathname,     /*!< Absolute pathname to the Media file.*/
                                                /*!< If NULL, then indicates that media info meta data can only be read from existing media Info file mentioned by pInfoFileAbsolutePathName, it fails if media info file doesn't exist.*/

    const char *pInfoFileAbsolutePathName,      /*!< Absolute path name to the Info file. If info file doesn't exist then acquire media info meta data and create media info file.*/
                                                /*!< If NULL, then indicates that acquire media info meta data for the file mentioned by pMediaFileAbsolutePathname.*/

    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings /*!< Optional address of a BIP_MediaInfoCreateSettings structure. */
                                                          /*!< Passing NULL will use default settings. */
    )
{
    BIP_Status rc    = BIP_SUCCESS;
    BIP_MediaInfoCreateSettings defaultMediaInfoCreateSettings;
    BIP_MediaInfoHandle hMediaInfo = NULL;

    BDBG_ENTER( BIP_MediaInfo_CreateFromMediaFile );
    BIP_SETTINGS_ASSERT(pMediaInfoCreateSettings, BIP_MediaInfoCreateSettings);

    if(NULL == pMediaInfoCreateSettings)
    {
        BIP_MediaInfo_GetDefaultCreateSettings(&defaultMediaInfoCreateSettings);
        pMediaInfoCreateSettings = &defaultMediaInfoCreateSettings;
    }

    hMediaInfo = B_Os_Calloc( 1, sizeof( BIP_MediaInfo ));
    BIP_CHECK_GOTO(( hMediaInfo != NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfo) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( hMediaInfo, BIP_MediaInfo );

    hMediaInfo->createSettings = *pMediaInfoCreateSettings;

    /* Create hAbsoluteMediaPath object, later it will be filled with data.*/
    hMediaInfo->hAbsoluteMediaPath = BIP_String_Create();
    BIP_CHECK_GOTO(( hMediaInfo->hAbsoluteMediaPath != NULL ), ( "Failed to allocate memory for BIP_String object"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    if(NULL == pMediaFileAbsolutePathname )
    {
        /* Check for info file name.*/
        if(NULL == pInfoFileAbsolutePathName )
        {
            rc = BIP_ERR_INVALID_PARAMETER;
            BDBG_ERR((BIP_MSG_PRE_FMT "Invalid parameter." BIP_MSG_PRE_ARG));
            goto error;
        }
        else
        {
            rc = BIP_MediaInfo_CheckForValidInfoFile(pInfoFileAbsolutePathName);
            BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_CheckForValidInfoFile failed" ), error, rc, rc );

            /* Valid media info file exist ,so retrive the meta data from existing info file. */
            BDBG_MSG(( BIP_MSG_PRE_FMT "Calling BIP_MediaInfo_RetriveMetaDataFromInfoFile to retrive medaiInfo meta data from existing info file " BIP_MSG_PRE_ARG));
            rc = BIP_MediaInfo_RetriveMetaDataFromInfoFile(
                     pInfoFileAbsolutePathName,
                     hMediaInfo
                     );
            BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_RetriveMetaDataFromInfoFile failed" ), error, rc, rc );
        }
    }
    else
    {
        /* Check whether infoFile exist.*/
        if(NULL != pInfoFileAbsolutePathName)
        {
            rc = BIP_MediaInfo_CheckForValidInfoFile(pInfoFileAbsolutePathName);
            if((BIP_SUCCESS != rc) || (true == hMediaInfo->createSettings.reAcquireInfo))
            {
                /* Reacquire the media info meta data from media file. So we internally set the reAcquire flag irrespective of whether it is set by app
                   in case when BIP_MediaInfo_CheckForValidInfoFile failed(since in bothe cases behaviour is same).On the basis of this flag we will save the new media info file.*/
                hMediaInfo->createSettings.reAcquireInfo = true;

                BDBG_MSG(( BIP_MSG_PRE_FMT "Calling BIP_MediaInfo_ProbeFileForMediaInfo to regenerate info file " BIP_MSG_PRE_ARG));

                rc = BIP_MediaInfo_ProbeFileForMediaInfo(
                                    pMediaFileAbsolutePathname,
                                    hMediaInfo
                                    );
                BIP_CHECK_GOTO(( rc==BIP_SUCCESS || rc == BIP_INF_MEDIA_INFO_UNKNOWN_CONTAINER_TYPE), ( "error Probing Stream(%s)", pMediaFileAbsolutePathname ), error, BIP_ERR_MEDIA_INFO_BAD_MEDIA_PATH, rc );
            }
            else
            {
                /* Valid media info file exist and reAcquire flag is not set so retrive the meta data from existing info file. */
                BDBG_MSG(( BIP_MSG_PRE_FMT "Calling BIP_MediaInfo_RetriveMetaDataFromInfoFile to retrive medaiInfo meta data from existing info file " BIP_MSG_PRE_ARG));
                rc = BIP_MediaInfo_RetriveMetaDataFromInfoFile(
                         pInfoFileAbsolutePathName,
                         hMediaInfo
                         );
                BIP_CHECK_GOTO(( rc == BIP_SUCCESS || rc == BIP_INF_MEDIA_INFO_VERSION_MISMATCH ), ( "BIP_MediaInfo_RetriveMetaDataFromInfoFile failed" ), error, rc, rc );

                if(rc == BIP_INF_MEDIA_INFO_VERSION_MISMATCH)
                {
                    /* Reacquire the media info meta data from media file since media info file is old and need to be updated.*/
                    hMediaInfo->createSettings.reAcquireInfo = true;

                    BDBG_WRN(( BIP_MSG_PRE_FMT "MediaInfo version mismatched, Calling BIP_MediaInfo_ProbeFileForMediaInfo to regenerate info file " BIP_MSG_PRE_ARG));

                    rc = BIP_MediaInfo_ProbeFileForMediaInfo(
                                    pMediaFileAbsolutePathname,
                                    hMediaInfo
                                    );
                    BIP_CHECK_GOTO(( rc==BIP_SUCCESS || rc == BIP_INF_MEDIA_INFO_UNKNOWN_CONTAINER_TYPE), ( "error Probing Stream(%s)", pMediaFileAbsolutePathname ), error, BIP_ERR_MEDIA_INFO_BAD_MEDIA_PATH, rc );
                }
            }
        }
        else /*(NULL == pInfoFileAbsolutePathName) */
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "Calling BIP_MediaInfo_ProbeFileForMediaInfo to regenerate info file " BIP_MSG_PRE_ARG));
            rc = BIP_MediaInfo_ProbeFileForMediaInfo(
                                pMediaFileAbsolutePathname,
                                hMediaInfo
                                );
            BIP_CHECK_GOTO(( rc==BIP_SUCCESS || rc==BIP_INF_MEDIA_INFO_UNKNOWN_CONTAINER_TYPE), ( "error Probing Stream(%s)", pMediaFileAbsolutePathname ), error, BIP_ERR_MEDIA_INFO_BAD_MEDIA_PATH, rc );
        }
    }

    /*If BIP_INF_MEDIA_INFO_UNKNOWN_CONTAINER_TYPE still we want to generate an xml file with the mediaName and file size. Rest of all fields will be set to 0. */
    if( hMediaInfo->createSettings.reAcquireInfo)
    {
        /* Create xml tree to generate the new xml info file.*/
        rc = BIP_MediaInfo_CreateXmlTree(hMediaInfo, pInfoFileAbsolutePathName);
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_CreateXmlTree failed" ), error, rc, rc );
    }

    BDBG_LEAVE( BIP_MediaInfo_CreateFromMediaFile );
    return hMediaInfo;
error:
    if(hMediaInfo)
    {
        BIP_MediaInfo_Destroy(hMediaInfo);
    }
    BDBG_LEAVE( BIP_MediaInfo_CreateFromMediaFile );
    return NULL;
}

static void destroyTracks(BIP_MediaInfoStream *pMediaInfoStream)
{
    BIP_MediaInfoTrack   *pCurrentTrack =  NULL;
    BIP_MediaInfoTrack   *pNextTrack =  NULL;

    pCurrentTrack = pMediaInfoStream->pFirstTrackInfoForStream;
    while(pCurrentTrack)
    {
        if(pCurrentTrack->trackType == BIP_MediaInfoTrackType_eAudio)
        {
            if(pCurrentTrack->info.audio.pLanguage)
            {
                B_Os_Free(pCurrentTrack->info.audio.pLanguage);
            }

            if(pCurrentTrack->info.audio.hlsSessionEnabled)
            {
                if(pCurrentTrack->info.audio.containerSpecificInfo.hls.pHlsLanguageCode)
                {
                    B_Os_Free(pCurrentTrack->info.audio.containerSpecificInfo.hls.pHlsLanguageCode);
                }
                if(pCurrentTrack->info.audio.containerSpecificInfo.hls.pLanguageName)
                {
                    B_Os_Free(pCurrentTrack->info.audio.containerSpecificInfo.hls.pLanguageName);
                }
                if(pCurrentTrack->info.audio.containerSpecificInfo.hls.pGroupId)
                {
                    B_Os_Free(pCurrentTrack->info.audio.containerSpecificInfo.hls.pGroupId);
                }
            }
        }
        pNextTrack =  pCurrentTrack->pNextTrackForStream;
        B_Os_Free(pCurrentTrack);
        pCurrentTrack = pNextTrack;
    }
}

static void destroyTrackGroups(BIP_MediaInfoStream *pMediaInfoStream)
{
    BIP_MediaInfoTrackGroup *pCurrentTrackGroup = NULL;
    BIP_MediaInfoTrackGroup *pNextTrackGroup = NULL;

    pCurrentTrackGroup = pMediaInfoStream->pFirstTrackGroupInfo;
    while(pCurrentTrackGroup)
    {
        pNextTrackGroup =  pCurrentTrackGroup->pNextTrackGroup;
        B_Os_Free(pCurrentTrackGroup);
        pCurrentTrackGroup = pNextTrackGroup;
    }
}

void    BIP_MediaInfo_Destroy(
    BIP_MediaInfoHandle hMediaInfo
    )
{

    BDBG_ENTER( BIP_MediaInfo_Destroy );

    BDBG_ASSERT(hMediaInfo);
    BDBG_OBJECT_ASSERT(hMediaInfo,BIP_MediaInfo);

    /* Destroy track groups.*/
    destroyTrackGroups(&hMediaInfo->mediaInfoStream);

    destroyTracks(&hMediaInfo->mediaInfoStream);

    if(hMediaInfo->hAbsoluteMediaPath)
    {
        BIP_String_Destroy(hMediaInfo->hAbsoluteMediaPath);
    }

    BDBG_OBJECT_DESTROY( hMediaInfo, BIP_MediaInfo );
    B_Os_Free(hMediaInfo);
    BDBG_LEAVE( BIP_MediaInfo_Destroy );
}

/**
Summary:
This API allows caller to obtain a pointer to the BIP_MediaInfoStream structure.

Description:
Once the caller has the pointer to the BIP_MediaInfoStream structure, they
can use the linked lists to navigate to any TrackGroups/Tracks that belong to
the Stream.
**/
BIP_MediaInfoStream  *BIP_MediaInfo_GetStream(
    BIP_MediaInfoHandle     hMediaInfo         /*!< Handle of the MediaInfo object. */
    )
{
    BDBG_ASSERT(hMediaInfo);
    BDBG_OBJECT_ASSERT(hMediaInfo,BIP_MediaInfo);

    BDBG_MSG((BIP_MSG_PRE_FMT "&(hMediaInfo->mediaInfoStream) = %p" BIP_MSG_PRE_ARG, &hMediaInfo->mediaInfoStream));

    return(&(hMediaInfo->mediaInfoStream));
}

/**
Summary:
Return a pointer to the TrackGroup structure for the specified TrackGroupId.

Return:
    non-NULL           : The requested pointer to the BIP_MediaInfoTrackGroup structure.
Return:
    NULL               : The specified trackGroupId does not exist.
**/
BIP_MediaInfoTrackGroup *BIP_MediaInfo_GetTrackGroupById(
    BIP_MediaInfoHandle     hMediaInfo,      /*!< Handle of the MediaInfo object. */
    unsigned                trackGroupId    /*!< The identifier of the requested TrackGroup. */
    )
{
    BIP_MediaInfoStream  *pMediaInfoStream = NULL;
    BIP_MediaInfoTrackGroup   **ppTrackGroup = NULL;

    BDBG_ASSERT(hMediaInfo);
    BDBG_OBJECT_ASSERT(hMediaInfo,BIP_MediaInfo);

    pMediaInfoStream = &(hMediaInfo->mediaInfoStream);
    ppTrackGroup = &(pMediaInfoStream->pFirstTrackGroupInfo);
    while(*ppTrackGroup)
    {
        if((*ppTrackGroup)->trackGroupId == trackGroupId)
        {
            break;
        }
        ppTrackGroup = &((*ppTrackGroup)->pNextTrackGroup);
    }
    return(*ppTrackGroup);
}
/**
Summary:
Return a pointer to the Track structure for the specified trackId.

Return:
    non-NULL           : The requested pointer to the BIP_MediaInfoTrack structure.
Return:
    NULL               : The specified trackId does not exist.
**/
BIP_MediaInfoTrack *BIP_MediaInfo_GetTrackById(
    BIP_MediaInfoHandle     hMediaInfo,      /*!< Handle of the MediaInfo object. */
    unsigned                trackId         /*!< The identifier of the requested Track. */
    )
{
    BIP_MediaInfoStream  *pMediaInfoStream = NULL;
    BIP_MediaInfoTrack   **ppTrack = NULL;

    BDBG_ASSERT(hMediaInfo);
    BDBG_OBJECT_ASSERT(hMediaInfo,BIP_MediaInfo);

    pMediaInfoStream = &(hMediaInfo->mediaInfoStream);
    ppTrack = &(pMediaInfoStream->pFirstTrackInfoForStream);

    while(*ppTrack)
    {
        if((*ppTrack)->trackId == trackId)
        {
            break;
        }
        ppTrack = &((*ppTrack)->pNextTrackForStream);
    }
    return (*ppTrack);
}

/******************************************************************************
 *
 * Summary:
 * Low-level API to make a Nav file for a specific program in an MPEG-2
 * Transport Stream file.
 *
 * Description:
 *
 ******************************************************************************/
BIP_Status
BIP_MediaInfo_MakeNavForTsFile(
    const char * pMediaFileAbsolutePathName,     /* Existing TS media file. */
    const char * pMediaNavFileAbsolutePathName,  /* Where to put new Nav file. If file exists, it not be overwritten. */
    unsigned     trackId,                        /* Video track PID */
    const BIP_MediaInfoMakeNavForTsSettings *pMakeNavForTsSettings)
{
    BIP_Status              rc;
    BIP_MediaInfoHandle hMediaInfo = NULL;
    BIP_MediaInfoTrack *pMediaTrack = NULL;

    BIP_SETTINGS_ASSERT(pMakeNavForTsSettings, BIP_MediaInfoMakeNavForTsSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry: ******************************************" BIP_MSG_PRE_ARG));
    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry: mediaFile=\"%s\"" BIP_MSG_PRE_ARG, pMediaFileAbsolutePathName));
    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry: navFile=\"%s\"" BIP_MSG_PRE_ARG, pMediaNavFileAbsolutePathName));
    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry: trackId=%d" BIP_MSG_PRE_ARG, trackId));

    /* Create mediaInfo object and probe for mediaInfo meta data.*/
    {
        hMediaInfo = B_Os_Calloc( 1, sizeof( BIP_MediaInfo ));
        BIP_CHECK_GOTO(( hMediaInfo != NULL ), ( "Failed to allocate memory (%d bytes) for BIP_MediaInfo Object", sizeof(BIP_MediaInfo) ), cleanup_probe, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        BDBG_OBJECT_SET( hMediaInfo, BIP_MediaInfo );

        BIP_MediaInfo_GetDefaultCreateSettings(&hMediaInfo->createSettings);

        /* Create hAbsoluteMediaPath object, later it will be filled with data.*/
        hMediaInfo->hAbsoluteMediaPath = BIP_String_Create();
        BIP_CHECK_GOTO(( hMediaInfo->hAbsoluteMediaPath != NULL ), ( "Failed to allocate memory for BIP_String object"), cleanup_probe, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );


        rc = BIP_MediaInfo_ProbeFileForMediaInfo(
                                    pMediaFileAbsolutePathName,
                                    hMediaInfo
                                    );
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_MediaInfo_ProbeFileForMediaInfo failed" ), cleanup_probe, rc, rc );
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Probe complete, TransportType=\"%s\""
               BIP_MSG_PRE_ARG, BIP_ToStr_NEXUS_TransportType(hMediaInfo->mediaInfoStream.transportType)));

    /* If media is a transport stream, try to generate a Nav file. */
    if ( hMediaInfo->mediaInfoStream.transportType == NEXUS_TransportType_eTs )
    {
        /* coverity[stack_use_local_overflow] */
        /* coverity[stack_use_overflow] */
        B_PlaybackIpPsiInfo                     bpip_psi;
        B_PlaybackIpFileStreamingOpenSettings   fileStreamingSettings;
        B_PlaybackIpFileStreamingHandle         fileStreamingHandle = NULL;
        bool        foundValidVideoTrack = false;

        /* Loop through all the stream's tracks (pids), looking for the requested video track (pid). */
        /*for (i = 0; i < psi.totalTracks; i++)*/

        for(pMediaTrack = hMediaInfo->mediaInfoStream.pFirstTrackInfoForStream; pMediaTrack; pMediaTrack = pMediaTrack->pNextTrackForStream)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "trackId %d has type:%s "
                       BIP_MSG_PRE_ARG,
                       pMediaTrack->trackId,
                       BIP_ToStr_BIP_MediaInfoTrackType(pMediaTrack->trackType)));

            /* If track's trackId (pid) isn't the one we want, skip it... */
            if (pMediaTrack->trackId != trackId) continue;

            /* If track's type isn't Video (it might be PCR), skip it... */
            if (pMediaTrack->trackType != BIP_MediaInfoTrackType_eVideo) continue;

            if(pMediaTrack->parsedPayload == 0) continue;/* This will take care of cases where pid exist but there is no actual content available for those tracks.*/

            BDBG_MSG(( BIP_MSG_PRE_FMT "Found Video trackId %d with Video codec: %s"
                    BIP_MSG_PRE_ARG,
                    pMediaTrack->trackId,
                    BIP_ToStr_NEXUS_VideoCodec(pMediaTrack->info.video.codec)));

            /* Check for codecs that we don't support for Nav files. */
            if (( pMediaTrack->info.video.codec == NEXUS_VideoCodec_eH264_Svc ) ||
                ( pMediaTrack->info.video.codec == NEXUS_VideoCodec_eH264_Mvc )) {

                BDBG_WRN(( BIP_MSG_PRE_FMT "trackId %d Video codec: %s is unsupported for Nav "
                           BIP_MSG_PRE_ARG,
                           pMediaTrack->trackId,
                           BIP_ToStr_NEXUS_VideoCodec(pMediaTrack->info.video.codec)));
                continue;   /* Can't do this codec, skip the track... */
            }

            /* If we've gotten this far, the track has passed all the checks.  Lets use this one. */
            B_Os_Memset( &bpip_psi, 0, sizeof( bpip_psi ));
            bpip_psi.videoPid       = pMediaTrack->trackId;
            bpip_psi.videoCodec     = pMediaTrack->info.video.codec;
            bpip_psi.mpegType       = hMediaInfo->mediaInfoStream.transportType;
            bpip_psi.transportTimeStampEnabled = hMediaInfo->mediaInfoStream.transportTimeStampEnabled;
            bpip_psi.videoFrameRate = (pMediaTrack->info.video.frameRate/1000);
            bpip_psi.duration       = hMediaInfo->mediaInfoStream.durationInMs;
            foundValidVideoTrack    = true;
            break;
        } /* End of loop for each track in stream. */

        BIP_CHECK_GOTO((foundValidVideoTrack), ("ERROR: Could not find valid/supported Video Track to create Nav file "),
                       cleanup_ts_nav, BIP_ERR_NOT_SUPPORTED, rc );

        /* playback ip only allows the info files to be created same place media file is.  */
        B_Os_Memset( &fileStreamingSettings, 0, sizeof( fileStreamingSettings ));
        fileStreamingSettings.generateAllProgramsInfoFiles = false;
        fileStreamingSettings.disableIndexGeneration       = false;
        fileStreamingSettings.disableHlsPlaylistGeneration = true;
        strncpy( fileStreamingSettings.fileName, pMediaFileAbsolutePathName, sizeof( fileStreamingSettings.fileName )-1 );
        strncpy( fileStreamingSettings.mediaInfoFilesDir, "", sizeof( fileStreamingSettings.mediaInfoFilesDir )-1 ); /* where info nav files should be */

        fileStreamingHandle = B_PlaybackIp_FileStreamingOpen( &fileStreamingSettings );
        BIP_CHECK_GOTO(( fileStreamingHandle != NULL ), ("ERROR: Failed to open File Streaming handle"),
                       cleanup_ts_nav, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        rc = openMediaIndexer( fileStreamingHandle, pMediaNavFileAbsolutePathName, &bpip_psi );
        BIP_CHECK_GOTO((rc), ("Failed to obtain Nav File for this media file (%s) \n", fileStreamingSettings.fileName),
                       cleanup_ts_nav, BIP_ERR_INVALID_PARAMETER, rc );

        rc = BIP_SUCCESS;

cleanup_ts_nav:
        B_PlaybackIp_FileStreamingClose( fileStreamingHandle );
    }
    else
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "Can't make Nav for transport type %s, skipping file: %s"
                   BIP_MSG_PRE_ARG, BIP_ToStr_NEXUS_TransportType(pMediaTrack->trackType), pMediaFileAbsolutePathName));
        rc =  BIP_ERR_NOT_SUPPORTED;
    }

cleanup_probe:

    if(hMediaInfo)
    {
        BIP_MediaInfo_Destroy(hMediaInfo);
    }
    return(rc);
}
