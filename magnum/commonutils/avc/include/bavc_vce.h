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
 ******************************************************************************/

#ifndef BAVC_VCE_H__
#define BAVC_VCE_H__

#include "bavc_vce_meta.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/*****************/
/* COMMON FIELDS */
/*****************/

/* COMMON field validity flags */
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID         0x00000001
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID                 0x00000002
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID                0x00000004
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID         0x00000008
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID                 0x00000010
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_STCSNAPSHOT_VALID         0x00000020
/* Add more field validity flags here */

/* COMMON indicator flags */
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START               0x00010000
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS                       0x00020000
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EMPTY_FRAME               0x00040000 /* PTS and ESCR are expected to be valid. LENGTH is expected to be 0. */
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_END                 0x00080000 /* If both FRAME_START and FRAME_END are set, then the descriptor references a complete frame */
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOC                       0x00100000 /* End Of Chunk indicator.  If set, then indicates this frame is the last frame of the FNRT chunk */
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FIRST_FRAME               0x00200000 /* First Frame indicator.  If set, then indicates this frame is the first frame from the encoder */
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SEGMENT_START             0x00400000 /* Segment Start indicator.  If set, then indicates this frame is the first frame of the segment */

/* Add more indicator flags ABOVE this line */
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA                  0x40000000
#define BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EXTENDED                  0x80000000

typedef struct BAVC_CompressedBufferDescriptor
{
   uint32_t uiFlags;

   /* Timestamp Parameters */
   uint32_t uiOriginalPTS; /* 32-bit original PTS value (in 45 Khz or 27Mhz?) */
   uint64_t uiPTS; /* 33-bit PTS value (in 90 Khz) */
   uint64_t uiSTCSnapshot; /* 42-bit STC snapshot when frame received by the encoder (in 27Mhz) */

   /* Transmission Parameters */
   uint32_t uiESCR; /* Expected mux transmission start time for the first bit of the data (in 27 Mhz) */

   /* TicksPerBit and iSHR indicate the transmission speed for the data.  So, the total duration (in 27 Mhz)
    * for the transmission of this buffer is:
    *
    * uiDeltaESCR = ( uiTicksPerBit * ( uiLength * 8 ) );
    * if ( iSHR > 0 )
    * {
    *    uiDeltaESCR >>= iSHR;
    * }
    * else
    * {
    *    uiDeltaESCR <<= -iSHR;
    * }
    *
    * And the *earliest* valid transmission start time for the next buffer is simply:
    *
    * uiNextESCR = uiESCR + uiDeltaESCR;
    */
   uint16_t uiTicksPerBit;
   int16_t iSHR;

   /* Buffer Parameters */
   unsigned uiOffset; /* REQUIRED: offset of frame data from frame buffer base address (in bytes) */
   unsigned uiLength;   /* REQUIRED: 0 if fragment is empty, e.g. for EOS entry (in bytes) */
   unsigned uiReserved[2]; /* Unused field */
} BAVC_CompressedBufferDescriptor;

typedef struct BAVC_CompressedBufferStatus
{
      /* Note: The following are CACHED addresses.
       *
       * The encoder must guarantee cache coherence for the referenced data.
       * The consumer must convert to uncached or offset as needed if the data will
       * be consumed by HW.
       */
      const void *pFrameBufferBaseAddress; /* [DEPRECATED - Set hFrameBufferBlock] The cached address of the start of the CDB. */
      const void *pMetadataBufferBaseAddress; /* [DEPRECATED - Set hMetadataBufferBlock] The cached address of the start of the metadata */
      BMMA_Block_Handle hFrameBufferBlock; /* The BMMA block for the CDB. */
      BMMA_Block_Handle hMetadataBufferBlock; /* The BMMA block for the metadata */
      BMMA_Block_Handle hIndexBufferBlock; /* The BMMA block for the ITB. */

      struct
      {
         unsigned uiDepth; /* The amount of data in the buffer (in bytes) */
         unsigned uiSize; /* The total size of the buffer (in bytes). If 0, then the depth is invalid. */
         uint32_t uiRead;  /* Address of the buffer READ register */
         uint32_t uiBase;  /* Address of the buffer BASE register */
         uint32_t uiValid; /* Address of the buffer VALID register */
         uint32_t uiEnd;   /* Address of the buffer END register */
      } stITB, stCDB;
      bool bReady; /* Set to true when the buffer has been initialized and ready to be consumed.
                    * Once set to true, it is always true for the duration of the session. */
} BAVC_CompressedBufferStatus;

/*************************/
/* VIDEO SPECIFIC FIELDS */
/*************************/

/* VIDEO field validity flags */
#define BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID                 0x00000001

/* VIDEO indicator flags */
#define BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP                       0x00010000
/* indicates a video data unit (NALU, EBDU, etc) starts at the beginning of
   this descriptor  - if this is set, then the uiDataUnitID field is valid also */
#define BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START           0x00020000

/* Only used for VP9: used to indicate this frame is a "hidden frame"
   within a superframe - the timestamp used for the superframe is the next
   non-hidden frame */
#define BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_HIDDEN_FRAME              0x00040000
#define BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_SHOW_FRAME                0x00080000

#define BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_EXTENDED                  0x80000000

typedef struct BAVC_VideoBufferDescriptor
{
      BAVC_CompressedBufferDescriptor stCommon;
      uint32_t uiVideoFlags;

      /* Timestamp Parameters */
      uint64_t uiDTS; /* 33-bit DTS value (in 90 Kh or 27Mhz?) */

      /* Metadata */
      uint8_t uiDataUnitType; /* The meaning of this field depends on the flags that are
                               * set in stCommon.uiFlags or uiVideoFlags.
                               * These flags are *mutually exclusive*.
                               *
                               * uiVideoFlags = BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START
                               *  protocol-specific data unit identifier. Contains the 1st byte of
                               *  the data unit (e.g. for H.264, this contains the 1st byte of the NALU:
                               *  nal_ref_idc=uiDataUnitType[6:5] and nal_unit_type=uiDataUnitType[4:0])
                               *
                               * uiFlags = BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA
                               *  indicates the type of metadata that is contained in the buffer.
                               *  See BAVC_VideoMetadataType enum for possible types and values
                               */
} BAVC_VideoBufferDescriptor;

typedef struct BAVC_VideoBufferStatus
{
      BAVC_CompressedBufferStatus stCommon;
} BAVC_VideoBufferStatus;

#define BAVC_VideoContextMap BAVC_XptContextMap

/***************************/
/* VIDEO Closed Captioning */
/***************************/
typedef struct BAVC_VideoUserDataStatus
{
        unsigned uiPendingBuffers; /* Buffers pending */
        unsigned uiCompletedBuffers; /* Buffers completed since previous call to XXX_GetStatusUserDataBuffers_isr() */
} BAVC_VideoUserDataStatus;

/**************/
/* VIP in VDC */
/**************/
typedef enum BAVC_VCE_BufferType
{
   BAVC_VCE_BufferType_eOriginal,
   BAVC_VCE_BufferType_eDecimated,
   BAVC_VCE_BufferType_eShiftedChroma,

   BAVC_VCE_BufferType_eMax
} BAVC_VCE_BufferType;

typedef struct BAVC_VCE_BufferConfig
{
   BAVC_ScanType eScanType;
   unsigned uiNumberOfBFrames;
} BAVC_VCE_BufferConfig;

void BAVC_VCE_GetDefaultBufferConfig_isrsafe(
      bool bInterlaced,
      BAVC_VCE_BufferConfig *pstBufferConfig
      );

unsigned BAVC_VCE_GetRequiredBufferCount_isrsafe(
      const BAVC_VCE_BufferConfig *pstBufferConfig,
      BAVC_VCE_BufferType eBufferType
      );

#ifdef __cplusplus
}
#endif

#endif /* BAVC_VCE_H__ */
