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
#ifndef BIP_MEDIA_INFO_PRIV_H
#define BIP_MEDIA_INFO_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
BIP_Media object is generated from a media stream. Media stream source can be a file
in hard disk or from a live source like qam tuner, bfile etc.
**/
typedef struct BIP_MediaInfo
{
    BDBG_OBJECT(BIP_MediaInfo)
    BIP_MediaInfoType     mediaInfoType;
    int64_t               sizeInBytes;             /*!< File size in bytes, 0 if not known or for Live Channel */
    BIP_StringHandle      hAbsoluteMediaPath;      /*!< complete path to media file, only for File Src */
    NEXUS_ParserBand      parserBand;              /*!< parserBand being used for the live channel */

    BIP_MediaInfoCreateSettings createSettings;

    /*BIP_StringHandle           hInfoPath;*/              /*!< complete path to info file */
    BIP_MediaInfoStream      mediaInfoStream;      /*!< BIP_MediaStream structure. */

    BIP_XmlElement  xmlElemRoot;

    int      playSpeed[IP_PVR_PLAYSPEED_MAX_COUNT];
    unsigned numPlaySpeedEntries;

    /* char   *playSpeedString;     implement later */                                     /* provides the playSpeed in the string format to allow receiving fractional speeds */
    /* bool                usePlaypump2ForAudio; figure out where this goes */  /* true => open audio pidChannel on nexusHandles.playpump2 */

} BIP_MediaInfo;

/**
Summary:
Create a BIP_MediaInfo object from a bmedia_probe_stream.

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
BIP_MediaInfoHandle BIP_MediaInfo_CreateFromBMediaStream_priv(
    const bmedia_probe_stream   *pStream,
    unsigned                    durationInMs,
    int64_t                     contentLength,
    bool                        liveChannel,
    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings   /*!< Optional address of a BIP_MediaInfoCreateSettings structure.  */
                                                            /*!< Passing NULL will use default settings.*/
    );

/**
Summary:
Create a BIP_MediaInfo object from a B_PlaybackIpPsiInfo.

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
BIP_MediaInfoHandle BIP_MediaInfo_CreateFromPbipPsi_priv(
    const B_PlaybackIpPsiInfo *pPsi,
    BIP_MediaInfoCreateSettings *pMediaInfoCreateSettings   /*!< Optional address of a BIP_MediaInfoCreateSettings structure.  */
                                                            /*!< Passing NULL will use default settings.*/
    );

BIP_Status BIP_MediaInfo_CreateXmlTree(
    BIP_MediaInfoHandle hMediaInfoHandle,
    const char *pInfoFileAbsolutePathName
    );

BIP_Status BIP_MediaInfo_CheckForValidInfoFile(
     const char *pInfoFileAbsolutePathName
     );

#ifdef __cplusplus
}
#endif

#endif /* BIP_MEDIA_INFO_PRIV_H */
