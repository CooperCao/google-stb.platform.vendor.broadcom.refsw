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

#ifndef BIP_MEDIA_INFO_NEW_H
#define BIP_MEDIA_INFO_NEW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip.h"
#include "bfile_io.h"

/** @addtogroup bip_media_info

BIP_MediaInfo Interface Definition.

The BIP_MediaInfo class is responsible for examining media to acquire its MediaInfo (MediaInformation or metadata),
and providing an API for accessing that MediaInfo.  It also provides for persistent filesystem based caching of
MediaInfo into an "Info file", allowing it to be accessed in the future without having to re-examine the source media.

When a MediaInfo object is created, it organizes the MediaInfo in a series of in-memory linked-lists,
and provides access to these lists with appropriate APIs.  Unless otherwise specified, one should not
presume that the linked lists are in any particular order.

\code
Here is a diagram that shows the linkages for a hypothetical stream with:
1. TrackGroup A: with Tracks AX, AY, AZ
2. TrackGroup B: with Tracks BX, BY, BZ
3. TrackGroup C: with no Tracks
4. Track X: not in any TrackGroup


 +--------------+          pFirstTrackInfoForStream
 |    Stream    |-----------------------------------------------+
 +--------------+                                               |
       |                                                        |
       | pFirstTrackGroupInfo                                   |
       V                                                        |
 +--------------+   pFirstTrackInfoforTrackGroup                |
 | TrackGroup A +-------------------------------------+         |
 +--------------+                                     |         |
       |                                              |         |
       |                                              V         V
       |                                        +-------------------+
       |                                        |  Track AX         |
       |                                        +-------------------+
       |                                              |         |
       |                  pNextTrackInfoForTrackGroup |         | pNextTrackForStream
       |                                              V         V
       | pNextTrackGroup                        +-------------------+
       |                                        |  Track AY         |
       |                                        +-------------------+
       |                                              |         |
       |                  pNextTrackInfoForTrackGroup |         | pNextTrackForStream
       |                                              V         V
       |                                        +-------------------+
       |                                        |  Track  AZ        |
       |                                        +-------------------+
       |                                              |         |
       |                  pNextTrackInfoForTrackGroup |         | pNextTrackForStream
       |                                              V         |
       |                                            [NULL]      |
       |                                                        V
       |                                        +-------------------+
       |                                        |  Track X          |
       |                                        |  (orphan - not in |
       |                                        |  any TrackGroup)  |
       |                                        +-------------------+
       |                                              |         |
       |                  pNextTrackInfoForTrackGroup |         | pNextTrackForStream
       |                                              V         |
       |                                            [NULL]      |
       V                                                        |
 +--------------+   pFirstTrackInfoforTrackGroup                |
 | TrackGroup B +-------------------------------------+         |
 +--------------+                                     |         |
       |                                              |         |
       | pNextTrackGroup                              |         |
       |                                              V         V
       |                                        +-------------------+
       |                                        |  Track BX         |
       |                                        +-------------------+
       |                                              |         |
       |                  pNextTrackInfoForTrackGroup |         | pNextTrackForStream
       |                                              V         V
       |                                        +-------------------+
       |                                        |  Track BY         |
       |                                        +-------------------+
       |                                              |         |
       |                  pNextTrackInfoForTrackGroup |         | pNextTrackForStream
       |                                              V         V
       |                                        +-------------------+
       |                                        |  Track BZ         |
       |                                        +-------------------+
       |                                              |         |
       |                  pNextTrackInfoForTrackGroup |         | pNextTrackForStream
       |                                              V         V
       |                                            [NULL]    [NULL]
       |
       |
       V
 +--------------+   pFirstTrackInfoforTrackGroup
 | TrackGroup C +---------------------------->[NULL]
 +--------------+
       |
       | pNextTrackGroup
       V
    [NULL]
\endcode
**/

#define BIP_MEDIA_INFO_VERSION  "02022016"   /* BIP_MediaInfo version is maintained in MMDDYYYY format.It will be updated whenever there is a structural change in Info file.*/

typedef struct BIP_MediaInfo *BIP_MediaInfoHandle;

/**
************************************************************************************************************************
Structure Definitions & APIs to retrieve Stream & Track Info associated with a various media inputs!
************************************************************************************************************************
**/

/**
Summary:
This enum defines mediaInfo type.
**/
typedef enum BIP_MediaInfoType
{
    BIP_MediaInfoType_eStream,                          /*!< Media type stream */
    BIP_MediaInfoType_eUnknown,                         /*!< Media type unknown */
    BIP_MediaInfoType_eMax
}BIP_MediaInfoType;

/**
Summary:
This enum defines commonly used tracks.
**/
typedef enum BIP_MediaInfoTrackType
{
    BIP_MediaInfoTrackType_eVideo,                       /*!< Video track */
    BIP_MediaInfoTrackType_eAudio,                       /*!< Audio track */
    BIP_MediaInfoTrackType_ePcr,                         /*!< Track with PCR information */
    BIP_MediaInfoTrackType_ePmt,                         /*!< Track with PMT information */
    BIP_MediaInfoTrackType_eOther,                       /*!< Track type other than listed above, it could be video or audio track with unknown codec type */
    BIP_MediaInfoTrackType_eMax
} BIP_MediaInfoTrackType;


/**
Summary:
This structure defines common properties of video track.
**/
typedef struct BIP_MediaInfoVideoTrack
{
    NEXUS_VideoCodec codec;                     /*!< Video codec */
    uint16_t         width;                     /*!< Coded video width, or 0 if unknown */
    uint16_t         height;                    /*!< Coded video height, or 0 if unknown  */
    unsigned         bitrate;                   /*!< Video bitrate in bps, or 0 if unknown */
    unsigned         frameRate;                 /*!< Video frame rate in 1000fps, or 0 if unknown */
    unsigned         colorDepth;                /*!< For H265/HEVC video code, color depth: 8 --> 8 bits, 10 --> 10 bits, 0 for other Video Codecs. */
} BIP_MediaInfoVideoTrack;

/**
Summary:
This describes Ac3 audio service type. Please  refer
to ATSC A/52:2012 Digital Audio Compression Standard spec
section 5.4.2.2 .
**/
typedef enum BIP_MediaInfoAudioAc3Bsmod
{
    BIP_MediaInfoAudioAc3Bsmod_eCompleteMain,     /*!< main audio service: complete main.*/
    BIP_MediaInfoAudioAc3Bsmod_eMusicAndEffects,  /*!< main audio service: music and effects.*/
    BIP_MediaInfoAudioAc3Bsmod_eVisuallyImpaired, /*!< main audio service: visually impaired.*/
    BIP_MediaInfoAudioAc3Bsmod_eHearingImpaired,  /*!< main audio service: hearing impaired.*/
    BIP_MediaInfoAudioAc3Bsmod_eDialogue,         /*!< main audio service: dialogue.*/
    BIP_MediaInfoAudioAc3Bsmod_eCommentary,       /*!< main audio service: commentary.*/
    BIP_MediaInfoAudioAc3Bsmod_eEmergency,        /*!< main audio service: emergency.*/
    BIP_MediaInfoAudioAc3Bsmod_eVoiceOver,        /*!< main audio service: voice over.*/
    BIP_MediaInfoAudioAc3Bsmod_eKaraoke,          /*!< main audio service: karaoke.*/
    BIP_MediaInfoAudioAc3Bsmod_eUnknown,          /*!< Can't determine bsmod value */
    BIP_MediaInfoAudioAc3Bsmod_eMax
}BIP_MediaInfoAudioAc3Bsmod;

/**
Summary:
Ac3 audio specific descriptor data extracted from PMT table.

Description:
These informations are usefull to enable decision making
on appropriate AC-3 stream(s). For ATSC Ac3 audio descriptor
details refer to ATSC A/52:2012 Digital Audio Compression
Standard spec section 4.
**/
typedef struct BIP_MediaInfoAudioAc3Descriptor
{
    bool                            bsmodValid;
    BIP_MediaInfoAudioAc3Bsmod      bsmod;       /*!< Describes the audio service type. Eg: main, music and effects, visually impaired etc. */
#if 0
    bool                            mainAudio;   /*!< This describes whether the audio is main audio or associated audio. Associated audio need to be presented with another main audio. */
    unsigned                        mainAudioId; /*!< If this audiotrack is main audio then this provides the main audio id. It can have a value in the range 0-7.*/
    uint8_t                         asvcflags;   /*!< This is required for associated audio.This is an 8-bit field . Each bit (0-7) indicates with which main service(s) this associated
                                                      audio can be presented. Eg: if bit-7 is set then that indicates that the associated audio can be presented with main audio with
                                                      mainAudioId == 7. */

    const char                      *language2;  /*!< This is only applicable for a dual mono stream where each channel can carry a different language audio.
                                                      If this is present then that indicates this audio track has two different language for left and right channel.
                                                      left channel contains a language described by "const char  *language"  in BIP_MediaInfoAudioTrack struct and righ channel
                                                      contains a language described by this variable.*/
#endif
}BIP_MediaInfoAudioAc3Descriptor;

#define BIP_MEDIA_INFO_LANGUAGE_FIELD_SIZE 4 /* As per ISO-639.2, 3byte char code and end of string. */

/*Following enum defined as per table 2.53, iso13818-1 */
typedef enum BIP_MediaInfoAudioMpegTsAudioType
{
    BIP_MediaInfoAudioMpegTsAudioType_eUndefined,
    BIP_MediaInfoAudioMpegTsAudioType_eCleanAffects,
    BIP_MediaInfoAudioMpegTsAudioType_eHearingImpaired,
    BIP_MediaInfoAudioMpegTsAudioType_eVisualImpairedCommentary,
    BIP_MediaInfoAudioMpegTsAudioType_eUnknown,                           /*!< Can't determine: how is it different from _eUndefined above?  */
    BIP_MediaInfoAudioMpegTsAudioType_eMax
}BIP_MediaInfoAudioMpegTsAudioType;

typedef struct BIP_MediaInfoAudioMpegTs
{
    BIP_MediaInfoAudioMpegTsAudioType audioType;
}BIP_MediaInfoAudioMpegTs;

typedef struct BIP_MediaInfoAudioHls
{
    bool        defaultAudio;                   /*!< If set, this entry should be the default Audio rendition to use if Player doesn't have a preference.*/
    unsigned    hlsAudioPid;                    /*!< pid associated with alternate audio rendition stream.*/
    char       *pHlsLanguageCode;               /*!< hls language code as revceived from hls data. This can be either in 3 byte or 2 byte code.*/
    char       *pLanguageName;                  /*!< Pointer to a const string containing language description. Eg: In hls, if language is set to "en" then languageName can be "English".*/
                                                /*!< For Hls descriptors please refer to hls spec section 8.7. It will be set to null incase language description doesn't exist.*/
    bool        requiresSecondPlaypumForAudio;  /*!< If true, then app must open & start a 2nd Nexus Playpump, open audio pidChannel & set nexusHandles.playpump2. */
    char       *pGroupId;                       /*!< Pointer to a null terminated string for an associated groupId. */
    NEXUS_TransportType hlsExtraAudioSpecificContainerType;
}BIP_MediaInfoAudioHls;

/**
Summary:
This structure defines common properties of audio track.
**/
typedef struct BIP_MediaInfoAudioTrack
{
    NEXUS_AudioCodec codec;                           /*!< Audio codec */
    uint8_t          channelCount;                    /*!< Number of channels, or 0 if unknown  */
    uint8_t          sampleSize;                      /*!< Number of bits in the each sample, or 0 if unknown */
    unsigned         bitrate;                         /*!< Audio bitrate in bps, or 0 if unknown */
    unsigned         sampleRate;                      /*!< Audio sampling rate in Hz, or 0 if unknown */
    char            *pLanguage;                       /*!< This points to a string containing 3-byte language code defining language */
                                                      /*!< of this audio service in ISO 639-2code. */
                                                      /*!< It will be set to NULL incase language doesn't exist. */
    bool             hlsSessionEnabled;               /*!< set if current session is being receiving using HTTP Live Streaming () Protocol, this has significant impact for audio tracks */
                                                      /*!< This should be cleaned once we do hls specific probing. */
    union
    {
        BIP_MediaInfoAudioMpegTs    mpegTs;           /*!< MpegTs specific audio related information. */
        BIP_MediaInfoAudioHls       hls;              /*!< HLS specific audio related information. */
    }containerSpecificInfo;

    union
    {
        BIP_MediaInfoAudioAc3Descriptor  ac3;         /*!< Ac3 audio descriptor data.*/
    }descriptor;                                      /*!< Codec specific descriptors. */
} BIP_MediaInfoAudioTrack;

/**
Summary:
Settings for BIP_MediaInfo_Create...().

See Also:
BIP_MediaInfo_GetDefaultCreateSettings
BIP_MediaInfo_CreateFromMediaFile
BIP_MediaInfo_CreateFromParserBand
BIP_MediaInfo_CreateFromBfile
BIP_MediaInfo_CreateFromUrl
BIP_MediaInfo_Destroy
**/
typedef struct BIP_MediaInfoCreateSettings
{
    BIP_SETTINGS(BIP_MediaInfoCreateSettings) /* Internal use... for init verification. */

    bool        reAcquireInfo;             /*!< Don't use existing info file (any cached data). Reacquire MediaInfo from the media source and create a new info file.*/

    int         psiAcquireTimeoutInMs;      /*!< In milliseconds(not Signal lock timeout), how long to wait for Psi  to acquire Pat/Pmt/etc.  */
    size_t      psiAcquireProbeSizeInBytes; /*!< Max amount of stream to read to obtain media info. */
} BIP_MediaInfoCreateSettings;
BIP_SETTINGS_ID_DECLARE(BIP_MediaInfoCreateSettings);

/**
Summary:
Get default settings for BIP_MediaInfo_Create...().

See Also:
BIP_MediaInfoCreateSettings
BIP_MediaInfo_CreateFromMediaFile
BIP_MediaInfo_CreateFromParserBand
BIP_MediaInfo_CreateFromBfile
BIP_MediaInfo_CreateFromUrl
BIP_MediaInfo_Destroy
**/
#define BIP_MediaInfo_GetDefaultCreateSettings(pSettings)                       \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_MediaInfoCreateSettings)  \
        /* Set non-zero defaults explicitly. */                                 \
        (pSettings)->psiAcquireTimeoutInMs = 2000;                              \
        (pSettings)->psiAcquireProbeSizeInBytes = 2 * 1024 *  1024;             \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Create a BIP_MediaInfo object from an existing media file.

Description:

Return:
    non-NULL : A BIP_MediaInfoHandle used for calling subsequent MediaInfo related APIs.
Return:
    NULL     : Failure, a BIP_MediaInfo instance could not be created.

See Also:
BIP_MediaInfoCreateSettings
BIP_MediaInfo_GetDefaultCreateSettings
BIP_MediaInfo_CreateFromParserBand
BIP_MediaInfo_CreateFromBfile
BIP_MediaInfo_CreateFromUrl
BIP_MediaInfo_Destroy
**/

BIP_MediaInfoHandle BIP_MediaInfo_CreateFromMediaFile(
    const char *pMediaFileAbsolutePathname,     /*!< Absolute pathname to the Media file.*/
                                                /*!< If NULL, the MediaInfo will be read from an existing Info file specified by pInfoFileAbsolutePathName. */

    const char *pInfoFileAbsolutePathName,      /*!< Absolute path name to the Info file. If info file doesn't exist then the MediaInfo will be acquired from the media file and stored in the Info file. */
                                                /*!< If NULL, the MediaInfoa will be acquired from the media file mentioned by pMediaFileAbsolutePathname. */

    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings /*!< Optional address of a BIP_MediaInfoCreateSettings structure. */
                                                          /*!< Passing NULL will use default settings. */
    );


/**
Summary:
Create a BIP_MediaInfo object from a NEXUS_ParserBand.

Description:

Return:
    non-NULL : A BIP_MediaInfoHandle used for calling subsequent MediaInfo related APIs.
Return:
    NULL     : Failure, a BIP_MediaInfo instance could not be created.

See Also:
BIP_MediaInfoCreateSettings
BIP_MediaInfo_GetDefaultCreateSettings
BIP_MediaInfo_CreateFromMediaFile
BIP_MediaInfo_CreateFromBfile
BIP_MediaInfo_CreateFromUrl
BIP_MediaInfo_Destroy
**/
BIP_MediaInfoHandle BIP_MediaInfo_CreateFromParserBand(
    NEXUS_ParserBand parserBand,                     /*!< parserBand connected to the media source. */
                                                     /*!< If NEXUS_ParserBand_eInvalid , the MediaInfo will be read from an existing media Info file specified by pInfoFileAbsolutePathName. */

    const char      *pInfoFileAbsolutePathName,      /*!< Absolute path name to the Info file. If info file doesn't exist then the MediaInfo will be acquired from the parserBand and stored in the Info file. */
                                                     /*!< If NULL, the MediaInfo will be acquired from the parserBand. */

    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings   /*!< Optional address of a BIP_MediaInfoCreateSettings structure.  */
                                                            /*!< Passing NULL will use default settings.*/
    );


/**
Summary:
Create a BIP_MediaInfo object from a bfile abstraction.

Description:

Return:
    non-NULL : A BIP_MediaInfoHandle used for calling subsequent MediaInfo related APIs.
Return:
    NULL     : Failure, a BIP_MediaInfo instance could not be created.

See Also:
BIP_MediaInfoCreateSettings
BIP_MediaInfo_GetDefaultCreateSettings
BIP_MediaInfo_CreateFromMediaFile
BIP_MediaInfo_CreateFromParserBand
BIP_MediaInfo_CreateFromUrl
BIP_MediaInfo_Destroy
**/
BIP_MediaInfoHandle BIP_MediaInfo_CreateFromBfile(
    bfile_io_read_t *pBfile,                         /*!< bfile handle attached to the media source. */
                                                     /*!< If NULL, the MediaInfo will be read from an existing media Info file specified by pInfoFileAbsolutePathName. */

    const char      *pInfoFileAbsolutePathName,      /*!< Absolute path name to the Info file. If info file doesn't exist then the MediaInfo will be acquired from the bfile and stored in the Info file. */
                                                     /*!< If NULL, the MediaInfo will be acquired from the bfile source. */

    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings   /*!< Optional address of a BIP_MediaInfoCreateSettings structure.  */
                                                            /*!< Passing NULL will use default settings.*/
    );

/**
Summary:
Create a BIP_MediaInfo object from a URL.

Description:

Return:
    non-NULL : A BIP_MediaInfoHandle used for calling subsequent MediaInfo related APIs.
Return:
    NULL     : Failure, a BIP_MediaInfo instance could not be created.

See Also:
BIP_MediaInfoCreateSettings
BIP_MediaInfo_GetDefaultCreateSettings
BIP_MediaInfo_CreateFromMediaFile
BIP_MediaInfo_CreateFromParserBand
BIP_MediaInfo_CreateFromBfile
BIP_MediaInfo_Destroy
**/
BIP_MediaInfoHandle BIP_MediaInfo_CreateFromUrl(
    const char  *pUrl,          /*!< URL as null-terminated char string, that refers to a media file.  */
                                /*!< If NULL, the MediaInfo will be read from an existing media Info file specified by pInfoFileAbsolutePathName. */

    const char  *pInfoFileAbsolutePathName,   /*!< Absolute path name to the Info file. If info file doesn't exist then the MediaInfo will be acquired from the URL and stored in the Info file. */
                                              /*!< If NULL, the MediaInfo will be acquired from the URL source. */

    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings   /*!< Optional address of a BIP_MediaInfoCreateSettings structure.  */
                                                            /*!< Passing NULL will use default settings.*/
    );


/**
Summary:
Destroy a BIP_MediaInfo object.

Description:
This will free all memory used for the MediaInfo object, then destry the object.
Any pointers returned by the BIP_MediaInfo APIs will become invalid.

Return:
    void

See Also:
BIP_MediaInfoCreateSettings
BIP_MediaInfo_GetDefaultCreateSettings
BIP_MediaInfo_CreateFromMediaFile
BIP_MediaInfo_CreateFromParserBand
BIP_MediaInfo_CreateFromBfile
BIP_MediaInfo_CreateFromUrl
**/
void BIP_MediaInfo_Destroy(
    BIP_MediaInfoHandle     hMediaInfo  /*!<   Handle of the BIP_MediaInfo object to be destroyed. */
    );


typedef struct BIP_MediaInfoStream   BIP_MediaInfoStream;
typedef struct BIP_MediaInfoTrackGroup   BIP_MediaInfoTrackGroup;
typedef struct BIP_MediaInfoTrack    BIP_MediaInfoTrack;

/**
Summary:
This structure provides top level information about a Stream such as its container format type, number of Tracks, and
other common info that applies across Tracks.
**/
struct BIP_MediaInfoStream
{
    BIP_MediaInfoTrackGroup       *pFirstTrackGroupInfo;       /*!< Pointer to first TrackGroup in TrackGroup list. */
    BIP_MediaInfoTrack        *pFirstTrackInfoForStream;   /*!< Pointer to first Track in Track list. */

    /* Info about the original media source. */
    const char *pMediaFileAbsolutePathname;        /*!< Full Media file pathname if MediaInfo was created from a MediaFile, otherwise NULL. */
    NEXUS_ParserBand parserBand;                   /*!< parserBand if MediaInfo was created from a ParserBand, otherwise NEXUS_ParserBand_eInvalid. */
    bfile_io_read_t *pBfile;                       /*!< Bfile descriptor if MediaInfo was created from a Bfile, otherwise NULL. */
    const char *pUrl;                              /*!< URL if MediaInfo was created from a URL, otherwise NULL  */

    NEXUS_TransportType transportType;                      /*!< Container format type */
    unsigned            numberOfTrackGroups;       /*!< Total number of TrackGroups in the Stream */
    unsigned            numberOfTracks;            /*!< Total number of Tracks in the Stream */
    unsigned            avgBitRate;                /*!< Average Stream bitrate in bits/second or 0 if unknown */
    unsigned            maxBitRate;                /*!< Maximum Stream bitrate in bits/second or 0 if unknown */
    unsigned            durationInMs;              /*!< Duration of Stream in milliseconds or 0 if unknown */
    int64_t             contentLength;             /*!< Content length of the File, 0 if not known or for Live Channel */
    bool                liveChannel;               /*!< If true, this Stream pertains to a Live Channel */
    bool                transportTimeStampEnabled; /*!< Indicates if MPEG2 TS content contains additional 4 byte timpstamp (192 byte Transport Packet) */
};


/**
Summary:
This structure defines TrackGroup-specific information.
**/
struct BIP_MediaInfoTrackGroup
{
    BIP_MediaInfoTrackGroup *pNextTrackGroup;          /*!< Pointer to next TrackGroup in the TrackGroup list. */
    BIP_MediaInfoTrack  *pFirstTrackForTrackGroup; /*!< Pointer to first Track in this TrackGroup's list. */

    unsigned trackGroupId;                 /*!< Unique TrackGroup identifier from Stream. */
                                           /*!<   For MPEG2-TS: program_number. */

    unsigned numberOfTracks;            /*!< Total number of Tracks in the TrackGroup */

    /** Container-specific TrackGroup information. */
    union {
        /** When BIP_MediaInfoStream.type == NEXUS_TransportType_eTs */
        struct {
            unsigned pmtPid;               /*!< For MPEG-TS: PID of Program Map Table */
        } Ts;
    } type;
};

/**
Summary:
This structure defines Track-specific information.
**/
struct BIP_MediaInfoTrack
{
    BIP_MediaInfoTrackType trackType;                /*!< Type of Track: Audio/Video/Pcr */
    unsigned               trackId;                  /*!< Unique Track ID (PID for MPEG2-TS, track_ID for ISOBMFF) */

    BIP_MediaInfoTrack   *pNextTrackForStream;     /*!< Pointer to next Track in Stream's Track list. */
    BIP_MediaInfoTrack   *pNextTrackForTrackGroup; /*!< Pointer to next Track in same TrackGroup. */
    BIP_MediaInfoTrackGroup *pParentTrackGroup;    /*!< Pointer to TrackGroup that this Track belongs to (or NULL). */

    size_t            parsedPayload;       /*!< Amount of payload (in bytes) encountered when parsing Stream */

#if 0
    /** Container-specific Track information. */
    union {
        /** When BIP_MediaInfoStream.type == NEXUS_TransportType_eTs */
        struct {
            size_t            parsedPayload;       /*!< Amount of payload (in bytes) encountered when parsing Stream */
        } Ts;
    } type;
#endif

    /** Track-type-specific Track information. */
    union {
        /** When BIP_MediaInfoTrack.type == BIP_MediaInfoTrackType_eAudio */
        BIP_MediaInfoAudioTrack audio;          /*!< Information for audio track */

        /** When BIP_MediaInfoTrack.type == BIP_MediaInfoTrackType_eVideo */
        BIP_MediaInfoVideoTrack video;          /*!< Information for video track */
    } info;
};

/**
Summary:
This API allows caller to obtain a pointer to the BIP_MediaInfoStream structure.

Description:
Once the caller has the pointer to the BIP_MediaInfoStream structure, they
can use the linked lists to navigate to any TrackGroups/Tracks that belong to
the Stream.

Return:
    non-NULL           : The requested pointer to the BIP_MediaInfoStream structure.
Return:
    NULL               : The BIP_MediaInfoStream structure is not available.

**/
BIP_MediaInfoStream  *BIP_MediaInfo_GetStream(
    BIP_MediaInfoHandle     hMediaInfo         /*!< Handle of the MediaInfo object. */
    );


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
    );


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
    );

/**
  * Summary:
  * used to mark if a navCreation was successful or not. Unknown marks nav creation  was not attempted
  **/
typedef enum BIP_MediaInfoNavCreated
{
    BIP_MediaInfoNavCreated_eUnknown,                       /* Unknown if nav not requested to be generated */
    BIP_MediaInfoNavCreated_eTrue,                          /* Nav Created success */
    BIP_MediaInfoNavCreated_eFalse,                         /* Nav Created failed*/
    BIP_MediaInfoNavCreated_eMax
} BIP_MediaInfoNavCreated;

/**
Summary:
Settings for BIP_MediaInfo_MakeNavForTsFile().

See Also:
BIP_MediaInfo_GetDefaultMakeNavForTsSettings
BIP_MediaInfo_MakeNavForTsFile
*/
typedef struct BIP_MediaInfoMakeNavForTsSettings
{
    BIP_SETTINGS(BIP_MediaInfoMakeNavForTsSettings) /* Internal use... for init verification. */

} BIP_MediaInfoMakeNavForTsSettings;
BIP_SETTINGS_ID_DECLARE(BIP_MediaInfoMakeNavForTsSettings);

/**
Summary:
Get default settings for BIP_MediaInfo_MakeNavForTsFile().

See Also:
BIP_MediaInfoMakeNavForTsSettings
BIP_MediaInfo_MakeNavForTsFile
**/
#define BIP_MediaInfo_GetDefaultMakeNavForTsSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_MediaInfoMakeNavForTsSettings) \
        /* Set non-zero defaults explicitly. */                                      \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Make a Nav file for a transport stream.

Description:
API to make a Nav file for a specific video track in an MPEG-2 Transport Stream file.

Return:
    BIP_SUCCESS  : A new Nav file has been created.

**/
BIP_Status
BIP_MediaInfo_MakeNavForTsFile(
    const char * pMediaFileAbsolutePathName,     /*!< Existing TS media file. */
    const char * pNavFileAbsolutePathName,       /*!< Where to put new Nav file. If file exists, it will be overwritten. */
    unsigned     trackId,                        /*!< The Video PID from which the Nav file should be generated. */
    const BIP_MediaInfoMakeNavForTsSettings *pMakeNavForTsSettings /*!< Optional address of a BIP_MediaInfoMakeNavForTsSettings structure.  */
                                                                     /*!< Passing NULL will use default settings.*/
    );
/**
Summary:
Print the BIP_MediaInfo related information on console.
**/
void BIP_MediaInfo_Print(BIP_MediaInfoHandle hMediaInfo);

#ifdef __cplusplus
}
#endif

#endif /* BIP_MEDIA_INFO_NEW_H */
