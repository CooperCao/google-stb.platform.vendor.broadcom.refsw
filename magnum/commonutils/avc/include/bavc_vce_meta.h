/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
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
} BAVC_VideoMetadataDescriptor;

#ifdef __cplusplus
}
#endif

#endif /* BAVC_VCE_META_H__ */
