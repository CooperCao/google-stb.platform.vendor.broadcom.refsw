/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BAVC_RAP_H__
#define BAVC_RAP_H__

#include "bavc_vce.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Audio compression standards

Description:
    Enumeration of the various Audio compression standards.
See Also:

****************************************************************************/
#ifndef BAVC_AUDIOCOMPRESSSIONSTD_DEFINED
typedef enum BAVC_AudioCompressionStd
{
    BAVC_AudioCompressionStd_eMpegL1,           /* MPEG Layer 1 */
    BAVC_AudioCompressionStd_eMpegL2,           /* MPEG Layer 2 */
    BAVC_AudioCompressionStd_eMpegL3,           /* MPEG Layer 3 */
    BAVC_AudioCompressionStd_eAac,              /* AAC (ADTS) */
    BAVC_AudioCompressionStd_eAacAdts=BAVC_AudioCompressionStd_eAac,            /* AAC ADTS */
    BAVC_AudioCompressionStd_eAacLoas,          /* AAC LOAS */
    BAVC_AudioCompressionStd_eAacPlus,          /* AAC Plus (HE/SBR) (LOAS) */
    BAVC_AudioCompressionStd_eAacPlusLoas=BAVC_AudioCompressionStd_eAacPlus,    /* AAC Plus (HE/SBR) LOAS */
    BAVC_AudioCompressionStd_eAacPlusAdts,      /* AAC Plus (HE/SBR) ADTS */
    BAVC_AudioCompressionStd_eAc3,              /* AC3 */
    BAVC_AudioCompressionStd_eAc3Plus,          /* AC3_PLUS */
    BAVC_AudioCompressionStd_eAc3Lossless,      /* AC3 LOSSLESS*/
    BAVC_AudioCompressionStd_eDts,              /* DTS */
    BAVC_AudioCompressionStd_eDtsHd,            /* DTSHD */
    BAVC_AudioCompressionStd_eDtshd=BAVC_AudioCompressionStd_eDtsHd,
    BAVC_AudioCompressionStd_eDtsCd,            /* DTS CD mode (14-bit/16-bit), uses legacy frame sync */
    BAVC_AudioCompressionStd_eDtsLegacy=BAVC_AudioCompressionStd_eDtsCd,
    BAVC_AudioCompressionStd_eDtsExpress,       /* DTS Express (DTS-LBR) */
    BAVC_AudioCompressionStd_eWmaStd,           /* WMA Standard */
    BAVC_AudioCompressionStd_eWmaStdTs,         /* WMA Standard with a 24-byte extended header */
    BAVC_AudioCompressionStd_eWmaPro,           /* WMA Pro */
    BAVC_AudioCompressionStd_eMlp,              /* MLP */
    BAVC_AudioCompressionStd_ePcm,              /* Raw PCM Data */
    BAVC_AudioCompressionStd_ePcmWav,           /* PCM input from a .wav source, requires header insertion */
    BAVC_AudioCompressionStd_eLpcmDvd,          /* DVD LPCM */
    BAVC_AudioCompressionStd_eLpcmHdDvd,        /* HD-DVD LPCM */
    BAVC_AudioCompressionStd_eLpcmBd,           /* Blu-Ray LPCM */
    BAVC_AudioCompressionStd_eLpcm1394,         /* IEEE 1394 LPCM */
    BAVC_AudioCompressionStd_eAmrNb,            /* Adaptive Multi-Rate compression (Narrow-Band) (typically used w/3GPP) */
    BAVC_AudioCompressionStd_eAmr=BAVC_AudioCompressionStd_eAmrNb,
    BAVC_AudioCompressionStd_eAmrWb,            /* Adaptive Multi-Rate compression (Wide-Band) (typically used w/3GPP) */
    BAVC_AudioCompressionStd_eDra,              /* Dynamic Resolution Adaptation.  Used in Blu-Ray and China Broadcasts. */
    BAVC_AudioCompressionStd_eCook,             /* Cook compression format, used in Real Audio 8 LBR */
    BAVC_AudioCompressionStd_eAdpcm,            /* MS ADPCM audio format */
    BAVC_AudioCompressionStd_eSbc,              /* Sub Band Codec used in Bluetooth A2DP audio */
    BAVC_AudioCompressionStd_eVorbis,           /* Vorbis audio codec.  Typically used with OGG or WebM container formats. */
    BAVC_AudioCompressionStd_eG711,             /* G.711 a-law and u-law companding.  Typically used for voice transmission. */
    BAVC_AudioCompressionStd_eG723_1,           /* G.723.1 Dual Rate Speech Coder for Multimedia Communications.  Used in H.324 and 3GPP 3G-324M.  This is different from G.723, which was superceded by G.726. */
    BAVC_AudioCompressionStd_eG726,             /* G.726 ADPCM speech codec.  Supercedes G.723 and G.721. */
    BAVC_AudioCompressionStd_eG729,             /* G.729 CS-ACELP speech codec.  Often used in VOIP applications. */
    BAVC_AudioCompressionStd_eFlac,             /* Free Lossless Audio Codec (see http://flac.sourceforge.net) */
    BAVC_AudioCompressionStd_eApe,              /* Monkey's Audio (see http://www.monkeysaudio.com/) */
    BAVC_AudioCompressionStd_eIlbc,             /* iLbc speech codec */
    BAVC_AudioCompressionStd_eIsac,             /* iSac speech codec */
    BAVC_AudioCompressionStd_eOpus,             /* Opus speech codec */
    BAVC_AudioCompressionStd_eAls,              /* MPEG-4 Audio Lossless Codec - ES */
    BAVC_AudioCompressionStd_eAlsLoas,          /* MPEG-4 Audio Lossless Codec - LOAS packed */
    BAVC_AudioCompressionStd_eAc4,              /* Dolby AC-4 Audio Codec */
    BAVC_AudioCompressionStd_eMax               /* Max value */
} BAVC_AudioCompressionStd;
#endif

/*************************/
/* AUDIO SPECIFIC FIELDS */
/*************************/
#define BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID                    0x00000001
#define BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_SAMPLING_FREQUENCY_VALID         0x00000002
#define BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_PROTOCOL_DATA_VALID              0x00000004
#define BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_TIMING_VALID                     0x00000008

#define BAVC_AUDIO_SPECIFIC_CONFIG_MAX_LENGTH (8)

typedef enum BAVC_AudioMetadataType
{
    BAVC_AudioMetadataType_eCommon, /* BAVC_AudioMetadataDescriptor */

    /* This enum cannot contain more than 256 entries because uiDataUnitType is defined as a uint8_t */
    BAVC_AudioMetadataType_eMax
} BAVC_AudioMetadataType;

typedef struct BAVC_AudioMetadataDescriptor
{
    uint32_t uiMetadataFlags;

    struct
    {
        unsigned uiMax; /* in bits/sec */
    } stBitrate;

    struct
    {
        unsigned uiSamplingFrequency;   /* In Hz */
    } stSamplingFrequency;

    struct
    {
        uint64_t uiSTCSnapshot; /* Initial 42-bit STC Snapshot from audio encode */
        unsigned uiChunkId; /* The FNRT chunk ID for the subsequent frame descriptors */
    } stTiming;

    BAVC_AudioCompressionStd eProtocol; /* Audio Compression Protocol */
    union
    {
        struct
        {
            uint8_t auiASC[BAVC_AUDIO_SPECIFIC_CONFIG_MAX_LENGTH];  /* Audio Specific Config from ISO 14496-3 */
            unsigned uiASCLengthBits; /* Length In Bits */
            unsigned uiASCLengthBytes;/* Length In Bytes - Since this is a bitfield extra bytes will be 0 filled */
        } stAac;                      /* Applies for BAVC_AudioCompressionStd_eAacAdts, BAVC_AudioCompressionStd_eAacLoas,
                                         BAVC_AudioCompressionStd_eAacPlusAdts, BAVC_AudioCompressionStd_eAacPlusLoas */
        struct
        {
            unsigned uiSamplesPerBlock;
            unsigned uiEncodeOptions;
            unsigned uiSuperBlockAlign;
            unsigned uiBlockAlign;
            unsigned uiNumChannels;
        } stWmaStd;                   /* Applies for BAVC_AudioCompressionStd_eWmaStd */
    } uProtocolData;
} BAVC_AudioMetadataDescriptor;

typedef struct BAVC_AudioBufferDescriptor
{
    BAVC_CompressedBufferDescriptor stCommon;

    /* Audio Specifics */

    /* Offset to RAW Frame Data and length. */
    unsigned uiRawDataOffset;  /* For most codecs, this will be equivalent to stCommon.uiOffset but if the data is encapsulated in another
                                  container (e.g. AAC ADTS) this will reflect the offset to the raw data block within the encapsulated
                                  frame. */
    size_t   uiRawDataLength;  /* For most codecs, this will be equivalent to stCommon.uiLength but if the data is encapsulated in another
                                  container (e.g. AAC ADTS) this will reflect the length of the raw data block within the encapsulated
                                  frame. */
    /* Metadata */
    uint8_t uiDataUnitType; /* If stCommon.uiFlags == BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA
                             *  this indicates the type of metadata that is contained in the buffer.
                             *  See BAVC_AudioMetadataType enum for possible types and values
                             */
} BAVC_AudioBufferDescriptor;

typedef struct BAVC_AudioBufferInfo
{
    BAVC_AudioCompressionStd eProtocol;
} BAVC_AudioBufferInfo;

typedef struct BAVC_AudioBufferStatus
{
    BAVC_CompressedBufferStatus stCommon;
} BAVC_AudioBufferStatus;

#define BAVC_AudioContextMap BAVC_XptContextMap


#ifdef __cplusplus
}
#endif

#endif /* BAVC_RAP_H__ */
