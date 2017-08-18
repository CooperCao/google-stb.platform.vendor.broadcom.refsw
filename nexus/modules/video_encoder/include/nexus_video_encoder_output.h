/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_VIDEO_ENCODER_OUTPUT_H__
#define NEXUS_VIDEO_ENCODER_OUTPUT_H__

#include "nexus_video_encoder_types.h"
#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
Summary:
Data returned by NEXUS_VideoEncoder_GetBuffer

Description:
This must match BAVC_VideoBufferDescriptor.
**/
typedef struct NEXUS_VideoEncoderDescriptor
{
    uint32_t flags;

    /* Timestamp Parameters */
    uint32_t originalPts; /* 32-bit original PTS value (in 45 KHz) */
    uint64_t pts; /* 33-bit PTS value (in 90 KHz) */
    uint64_t stcSnapshot; /* 42-bit STC snapshot when frame received by the encoder (in 27MHz) */

    /* Transmission Parameters */
    uint32_t escr; /* Expected mux transmission start time (in 27 MHz) */
    uint16_t ticksPerBit;
    int16_t shr;

    /* Buffer Parameters */
    unsigned offset; /* offset of frame data from frame buffer base address (in bytes) */
    size_t length;   /* 0 if fragment is empty, e.g. for EOS entry (in bytes) */
    unsigned unused[2];

    uint32_t videoFlags;

    uint64_t dts; /* 33-bit DTS value (in 90 Khz) */

    uint8_t dataUnitType;

} NEXUS_VideoEncoderDescriptor;

/**
Summary:
Flags for the NEXUS_VideoEncoderDescriptor.flags field
**/
 #define NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_ORIGINALPTS_VALID 0x00000001
 #define NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID 0x00000002
 #define NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_START 0x00010000
 #define NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS 0x00020000
 #define NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EMPTY_FRAME 0x00040000 /* PTS and ESCR are expected to be valid. LENGTH is expected to be 0. */
 #define NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_END 0x00080000 /* If both FRAME_START and FRAME_END are set, then the descriptor references a complete frame */
 #define NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOC 0x00100000 /* unused */
 #define NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA 0x40000000
 #define NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EXTENDED 0x80000000

/**
Summary:
Flags for the NEXUS_VideoEncoderDescriptor.videoFlags field
**/
 #define NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DTS_VALID 0x00000001
 #define NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_RAP 0x00010000
/**
Description:
 This flags indicates that a video data unit (NALU, EBDU, etc) starts at the beginning of
 this descriptor - if this is set, then the dataUnitType field is valid also
**/
 #define NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DATA_UNIT_START 0x00020000


/**
Summary:
The mux manager (or other consumer) will call this API to get encoded pictures from the NEXUS_VideoEncoder
**/
NEXUS_Error NEXUS_VideoEncoder_GetBuffer(
    NEXUS_VideoEncoderHandle handle,
    const NEXUS_VideoEncoderDescriptor **pBuffer, /* [out] attr{memory=cached} pointer to NEXUS_VideoEncoderDescriptor structs */
    size_t *pSize, /* [out] number of NEXUS_VideoEncoderPicture elements in pBuffer */
    const NEXUS_VideoEncoderDescriptor **pBuffer2, /* [out] attr{memory=cached} pointer to NEXUS_VideoEncoderDescriptor structs after wrap around */
    size_t *pSize2 /* [out] number of NEXUS_VideoEncoderPicture elements in pBuffer2 */
    );

/**
Summary:
Report how much data returned by last NEXUS_VideoEncoder_GetBuffer call was consumed
**/
NEXUS_Error NEXUS_VideoEncoder_ReadComplete(
    NEXUS_VideoEncoderHandle handle,
    unsigned descriptorsCompleted /* must be <= pSize+pSize2 returned by last NEXUS_VideoEncoder_GetPictures call. */
    );

/**
Summary:
The mux manager (or other consumer) will call this API to get encoded pictures from the NEXUS_VideoEncoder
Populates one video encoder descriptor for each frame into the provided NEXUS_VideoEncoderDescriptor array.
Each frame will only be returned once. I.e. Subsequent calls to ReadIndex will return only descriptors that
weren't read previously.

This API does not modify the index/data pointers, so can be called in conjunction with any other consumer.

The index is implemented as a circular FIFO, so will always return the newest descriptors when called.
To ensure no descriptors are missed, ReadIndex must be called periodically and repeatedly until  *pRead < size.
**/
NEXUS_Error NEXUS_VideoEncoder_ReadIndex(
    NEXUS_VideoEncoderHandle handle,
    NEXUS_VideoEncoderDescriptor *pBuffer, /* attr{nelem=size;nelem_out=pRead} [out] pointer to NEXUS_VideoEncoderDescriptor structs */
    unsigned size, /* max number of NEXUS_VideoEncoderPicture elements in pBuffer */
    unsigned *pRead /* [out] number of NEXUS_VideoEncoderPicture elements read */
    );

#define NEXUS_VIDEOENCODER_ERROR_INVALID_INPUT_DIMENSION          0x00000001
#define NEXUS_VIDEOENCODER_ERROR_USER_DATA_LATE                   0x00000002
#define NEXUS_VIDEOENCODER_ERROR_USER_DATA_DUPLICATE              0x00000004
#define NEXUS_VIDEOENCODER_ERROR_ADJUSTS_WRONG_FRAME_RATE         0x00000008
#define NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_BVN_FRAME_RATE       0x00000010
#define NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_RESOLUTION           0x00000020
#define NEXUS_VIDEOENCODER_ERROR_MISMATCH_BVN_MIN_FRAME_RATE      0x00000040
#define NEXUS_VIDEOENCODER_ERROR_MISMATCH_BVN_PIC_RESOLUTION      0x00000080
#define NEXUS_VIDEOENCODER_ERROR_MAX_BITRATE_EXCEEDED             0x00000100
#define NEXUS_VIDEOENCODER_ERROR_BIN_BUFFER_FULL                  0x00000200
#define NEXUS_VIDEOENCODER_ERROR_CDB_FULL                         0x00000400
#define NEXUS_VIDEOENCODER_ERROR_PICARC_TO_CABAC_DINO_BUFFER_FULL 0x00000800
#define NEXUS_VIDEOENCODER_ERROR_EBM_FULL                         0x00001000
#define NEXUS_VIDEOENCODER_ERROR_MAX_NUM_SLICES_EXCEEDED          0x00002000
#define NEXUS_VIDEOENCODER_ERROR_MAX_NUM_ENTRIES_INTRACODED_EXCEEDED   0x00004000
#define NEXUS_VIDEOENCODER_ERROR_IBBP_NOT_SUPPORTED_FOR_RESOLUTION 0x00008000
#define NEXUS_VIDEOENCODER_ERROR_MBARC_BOOT_FAILURE                0x00010000
#define NEXUS_VIDEOENCODER_ERROR_MEASURED_ENCODER_DELAY_TOO_LONG   0x00020000
#define NEXUS_VIDEOENCODER_ERROR_CRITICAL                          0x00040000
#define NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_3_CH_MODE  0x00080000
#define NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_2_CH_MODE  0x00100000
#define NEXUS_VIDEOENCODER_ERROR_MAX_RESOLUTION_FOR_LEVEL_EXCEEDED 0x00200000


#define NEXUS_VIDEOENCODER_EVENT_INPUT_CHANGE                     0x00000001
#define NEXUS_VIDEOENCODER_EVENT_EOS                              0x00000002


/**
Summary:
**/
typedef struct NEXUS_VideoEncoderStatus
{
    NEXUS_MemoryBlockHandle bufferBlock; /* block handle for NEXUS_VideoEncoderPicture.offset */
    NEXUS_MemoryBlockHandle metadataBufferBlock; /* block handle for NEXUS_VideoEncoderPicture.offset when it's used to carry metadata  */
    uint32_t errorFlags;
    uint32_t eventFlags;

    unsigned errorCount; /* Total number of errors that has occurred */
    unsigned picturesReceived; /* Number of pictures received at the input to the encoder */
    unsigned picturesDroppedFRC;  /* Number of pictures that the encoder has configured to drop in order to follow the requested
                             frame rate (Frame Rate Conversion) */
    unsigned picturesDroppedHRD; /* Number of pictures that the encoder has dropped because of a drop request from the Rate Control.
                             The Rate Control may decide to drop picture in order to maintain the HRD buffer model. */
    unsigned picturesDroppedErrors; /* Number of pictures that the encoder has configured to drop because encoder did not finish the
                             processing of the previous pictures on time and buffers are full. */
    unsigned picturesEncoded; /* Number of pictures output by the encoder */
    uint32_t pictureIdLastEncoded; /* Picture ID of the current picture being encoded. This is set as soon as the CME block decides to work on a picture. */
    unsigned picturesPerSecond; /* Averages pictures per second output by the encoder */
    struct {
        size_t fifoDepth; /* Current depth of video encoder output fifo in bytes */
        size_t fifoSize;  /* Size of video encoder output fifo in bytes. This could be less than the OpenSettings fifoSize because of
                             internal alignment requirements. */
    } data, index;
    struct {
        unsigned firmware;
    } version;
    NEXUS_DebugFifoInfo debugLog;
} NEXUS_VideoEncoderStatus;

/**
Summary:
**/
NEXUS_Error NEXUS_VideoEncoder_GetStatus(
    NEXUS_VideoEncoderHandle handle,
    NEXUS_VideoEncoderStatus *pStatus /* [out] */
    );



#ifdef __cplusplus
}
#endif


#endif /* NEXUS_VIDEO_ENCODER_OUTPUT_H__ */

