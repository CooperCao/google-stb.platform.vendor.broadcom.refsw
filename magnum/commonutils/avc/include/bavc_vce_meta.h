/***************************************************************************
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
 *
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BAVC_VCE_META_H__
#define BAVC_VCE_META_H__

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/******************/
/* VIDEO Metadata */
/******************/
#define BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID                0x00000001
#define BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_FRAMERATE_VALID              0x00000002
#define BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_DIMENSION_CODED_VALID        0x00000004
#define BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BUFFERINFO_VALID             0x00000008
#define BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_PROTOCOL_DATA_VALID          0x00000010
#define BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_STC_VALID             0x00000020
#define BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_CHUNK_ID_VALID        0x00000040
#define BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_ETS_DTS_OFFSET_VALID  0x00000080

typedef enum BAVC_VideoMetadataType
{
   BAVC_VideoMetadataType_eCommon, /* BAVC_VideoMetadataDescriptor */

   /* This enum cannot contain more than 256 entries because uiDataUnitType is defined as a uint8_t */
   BAVC_VideoMetadataType_eMax
} BAVC_VideoMetadataType;

typedef struct BAVC_VideoMetadata_VC1
{
      /* Sequence Header A (VERT_SIZE and HORIZ_SIZE): See BAVC_VideoMetadataDescriptor.stDimension.coded */

      /* Sequence Header B */
      struct
      {
            /* For LEVEL: See BAVC_VideoMetadataDescriptor.stBufferInfo.eLevel */
            bool bCBR; /* CBR */
            uint32_t uiHRDBuffer; /* HRD_BUFFER (Only 24 bits are valid) */
            /* For HRD_RATE: See BAVC_VideoMetadataDescriptor.stBitrate */
            /* For FRAMERATE: See BAVC_VideoMetadataDescriptor.stFrameRate */
      } sequenceHeaderB;

      struct
      {
            /* For Profile: See BAVC_VideoMetadataDescriptor.stBufferInfo.eProfile */
            uint8_t uiQuantizedFrameRatePostProcessing; /* FRMRTQ_POSTPROC (Only 3 bits are valid) */
            uint8_t uiQuantizedBitratePostProcessing; /* BITRTQ_POSTPROC (Only 5 bits are valid) */
            bool bLoopFilter; /* LOOPFILTER */
            bool bMultiResolution; /* MULTIRES */
            bool bFastUVMotionCompensation; /* FASTUVMC */
            bool bExtendedMotionVector; /* EXTENDED_MV */
            uint8_t uiMacroblockQuantization; /* DQUANT (Only 2 bits are valid) */
            bool bVariableSizedTransform; /* VSTRANSFORM */
            bool bOverlappedTransform; /* OVERLAP */
            bool bSyncMarker; /* SYNCMARKER */
            bool bRangeReduction; /* RANGERED */
            uint8_t uiMaxBFrames; /* MAXBFRAMES (Only 3 bits are valid) */
            uint8_t uiQuantizer; /* QUANTIZER (Only 2 bits are valid) */
            bool bFrameInterpolation; /* FINTERPFLAG */
      } sequenceHeaderC;
} BAVC_VideoMetadata_VC1;

typedef struct BAVC_VideoBufferInfo
{
      BAVC_VideoCompressionStd eProtocol;
      BAVC_VideoCompressionProfile eProfile;
      BAVC_VideoCompressionLevel eLevel;
} BAVC_VideoBufferInfo;

#define BAVC_VIDEO_MAX_EXTRACTED_NALU_SIZE 128
#define BAVC_VIDEO_MAX_EXTRACTED_NALU_COUNT 3

typedef struct BAVC_VideoExtractedNALUEntries
{
   uint8_t auiData[BAVC_VIDEO_MAX_EXTRACTED_NALU_SIZE];
   unsigned uiNumValidBytes;
} BAVC_VideoExtractedNALUEntry;

typedef struct BAVC_VideoExtractedNALUInfo
{
   BAVC_VideoExtractedNALUEntry astExtractedNALUEntry[BAVC_VIDEO_MAX_EXTRACTED_NALU_COUNT];
   unsigned uiNumExtractedNaluEntries;
} BAVC_VideoExtractedNALUInfo;

typedef struct BAVC_VideoMetadataDescriptor
{
      uint32_t uiMetadataFlags;

      BAVC_VideoBufferInfo stBufferInfo;

      struct
      {
            uint32_t uiMax; /* in bits/sec */
      } stBitrate;

      struct
      {
            BAVC_FrameRateCode eFrameRateCode;
      } stFrameRate;

      struct
      {
            struct
            {
                  uint16_t uiWidth;
                  uint16_t uiHeight;
            } coded;
      } stDimension;

      union
      {
            BAVC_VideoMetadata_VC1 stVC1; /* Applies for BAVC_VideoCompressionStd_eVC1SimpleMain */
      } uProtocolData;

      struct
      {
            uint64_t uiSTCSnapshot; /* Initial 42-bit STC Snapshot from video encode */
            unsigned uiChunkId; /* The FNRT chunk ID for the subsequent frame descriptors */
            unsigned uiEtsDtsOffset; /* The ETS to DTS offset for the encode session as determined by RC */
      } stTiming;

      BAVC_VideoExtractedNALUInfo stExtractedNALUInfo; /* If the video encoder ITB contains NALU data beyond the NAL unit header,
                                                        * then ALL of the NALU bytes are copied to this data structure. Each array will
                                                        * start with the NAL unit header. (1 byte for AVC, 2 bytes for HEVC, etc) */
} BAVC_VideoMetadataDescriptor;

#ifdef __cplusplus
}
#endif

#endif /* BAVC_VCE_META_H__ */
