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
#ifndef NEXUS_AUDIO_DSP_VIDEO_ENCODER_H__
#define NEXUS_AUDIO_DSP_VIDEO_ENCODER_H__

#include "bxudlib.h"

typedef struct NEXUS_DspVideoEncoder *NEXUS_DspVideoEncoderHandle;

typedef struct NEXUS_DspVideoEncoderOpenSettings
{
    unsigned unused;
} NEXUS_DspVideoEncoderOpenSettings;


typedef struct NEXUS_DspVideoEncoderExtInterruptInfo {
    uint32_t address;
    uint32_t bit_num;
} NEXUS_DspVideoEncoderExtInterruptInfo;

typedef struct NEXUS_DspVideoEncoderUserDataSettings {
    BXUDlib_Settings xudSettings;
    bool encodeUserData;
} NEXUS_DspVideoEncoderUserDataSettings;

typedef struct NEXUS_DspVideoEncoderStartSettings
{
    NEXUS_VideoCodec codec;
    BAVC_VideoCompressionProfile profile;
    BAVC_VideoCompressionLevel level;
    bool nonRealTime;
    NEXUS_VideoFrameRate framerate;
    uint32_t width;
    uint32_t height;
    uint32_t bitrate;
    uint32_t encoderDelay;
    NEXUS_RaveHandle raveContext;
    NEXUS_StcChannelHandle stcChannel;
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_DspVideoEncoderExtInterruptInfo extIntInfo;
#endif
    NEXUS_DspVideoEncoderUserDataSettings userDataSettings;
} NEXUS_DspVideoEncoderStartSettings;

typedef struct NEXUS_DspVideoEncoderSettings
{
    unsigned bitrate;
    NEXUS_VideoFrameRate frameRate;
} NEXUS_DspVideoEncoderSettings;
/**
Summary:
Data returned by NEXUS_DspVideoEncoder_GetBuffer

Description:
This must match BAVC_VideoBufferDescriptor.
**/
typedef struct NEXUS_DspVideoEncoderDescriptor
{
   uint32_t flags;

   /* Timestamp Parameters */
   uint32_t originalPTS; /* 32-bit original PTS value (in 45 Khz or 27Mhz?) */
   uint64_t pts; /* 33-bit PTS value (in 90 Khz or 27Mhz?) */
    uint64_t uiSTCSnapshot; /* 42-bit STC snapshot when frame received by the encoder (in 27Mhz) */

   /* Transmission Parameters */
   uint32_t escr; /* Expected mux transmission start time (in 27 Mhz) */
   uint16_t ticksPerBit;
   int16_t shr;

   /* Buffer Parameters */
   unsigned offset; /* offset of frame data from frame buffer base address (in bytes) */
   size_t length;   /* 0 if fragment is empty, e.g. for EOS entry (in bytes) */
   unsigned decoderIndex;
   unsigned unused;
   uint32_t videoFlags;

   uint64_t dts; /* 33-bit DTS value (in 90 Khz) */

   uint8_t dataUnitType;

} NEXUS_DspVideoEncoderDescriptor;

/**
Summary:
Flags for the NEXUS_VideoEncoderDescriptor.flags field
**/
 #define NEXUS_DSPVIDEOENCODERDESCRIPTOR_FLAG_ORIGINALPTS_VALID 0x00000001
 #define NEXUS_DSPVIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID 0x00000002
 #define NEXUS_DSPVIDEOENCODERDESCRIPTOR_FLAG_FRAME_START 0x00010000
 #define NEXUS_DSPVIDEOENCODERDESCRIPTOR_FLAG_EOS 0x00020000
 #define NEXUS_DSPVIDEOENCODERDESCRIPTOR_FLAG_METADATA 0x40000000

/**
Summary:
Flags for the NEXUS_VideoEncoderDescriptor.videoFlags field
**/
 #define NEXUS_DSPVIDEOENCODERDESCRIPTOR_VIDEOFLAG_DTS_VALID 0x00000001
 #define NEXUS_DSPVIDEOENCODERDESCRIPTOR_VIDEOFLAG_RAP 0x00010000
/**
Description:
 This flags indicates that a video data unit (NALU, EBDU, etc) starts at the beginning of
 this descriptor - if this is set, then the dataUnitType field is valid also
**/
 #define NEXUS_DSPVIDEOENCODERDESCRIPTOR_VIDEOFLAG_DATA_UNIT_START 0x00020000

typedef struct NEXUS_DspVideoEncoderStatus
{
    const void *bufferBase; /* attr{memory=cached} [deprecated] base pointer for NEXUS_VideoEncoderPicture.offset */
    const void *metadataBufferBase; /* attr{memory=cached} [deprecated] base pointer for NEXUS_VideoEncoderPicture.offset when it's used to carry metadata */
    NEXUS_MemoryBlockHandle bufferBlock; /* block handle for NEXUS_VideoEncoderPicture.offset */
    NEXUS_MemoryBlockHandle metadataBufferBlock; /* block handle for NEXUS_VideoEncoderPicture.offset when it's used to carry metadata  */
    uint32_t errorFlags;
    uint32_t eventFlags;

    unsigned errorCount;
    unsigned picturesReceived;
    unsigned picturesDroppedFRC;
    unsigned picturesDroppedErrors;
    unsigned picturesEncoded;
    uint32_t pictureIdLastEncoded;
    struct {
        unsigned firmware;
    } version;
} NEXUS_DspVideoEncoderStatus;

typedef struct NEXUS_DspVideoEncoderPicture {
    BMMA_Block_Handle   hImage;
    unsigned            offset;
    unsigned            width;
    unsigned            height;
    BAVC_Polarity       polarity;
    uint32_t            origPts;
    NEXUS_VideoFrameRate framerate;
    uint32_t            stallStc;
    uint32_t            ignorePicture;
    unsigned            sarHorizontal;
    unsigned            sarVertical;
} NEXUS_DspVideoEncoderPicture;


void NEXUS_DspVideoEncoder_GetDefaultStartSettings_priv( NEXUS_DspVideoEncoderStartSettings *pSettings );
void NEXUS_DspVideoEncoder_GetDefaultOpenSettings_priv( NEXUS_DspVideoEncoderOpenSettings *pSettings );
NEXUS_DspVideoEncoderHandle NEXUS_DspVideoEncoder_Open_priv(unsigned index, const NEXUS_DspVideoEncoderOpenSettings *openSettings);
void NEXUS_DspVideoEncoder_Close_priv(NEXUS_DspVideoEncoderHandle decoder);
void NEXUS_DspVideoEncoder_Release_priv(NEXUS_DspVideoEncoderHandle decoder);
NEXUS_Error NEXUS_DspVideoEncoder_Start_priv(NEXUS_DspVideoEncoderHandle decoder, const NEXUS_DspVideoEncoderStartSettings *startSettings);
void NEXUS_DspVideoEncoder_Stop_priv(NEXUS_DspVideoEncoderHandle decoder);
NEXUS_Error NEXUS_DspVideoEncoder_Flush_priv(NEXUS_DspVideoEncoderHandle decoder);
void NEXUS_DspVideoEncoder_GetRaveSettings_priv(NEXUS_RaveOpenSettings *raveSettings);
NEXUS_Error NEXUS_DspVideoEncoder_GetBuffer_priv(NEXUS_DspVideoEncoderHandle encoder, const NEXUS_DspVideoEncoderDescriptor **pBuffer, size_t *pSize, const NEXUS_DspVideoEncoderDescriptor **pBuffer2, size_t *pSize2);
NEXUS_Error NEXUS_DspVideoEncoder_ReadComplete_priv(NEXUS_DspVideoEncoderHandle encoder, unsigned descriptorsCompleted);
NEXUS_Error NEXUS_DspVideoEncoder_GetStatus_priv(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderStatus *pStatus);
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
NEXUS_Error NEXUS_DspVideoEncoder_EnqueuePicture_isr(NEXUS_DspVideoEncoderHandle encoder, BAVC_EncodePictureBuffer *picture);
NEXUS_Error NEXUS_DspVideoEncoder_DequeuePicture_isr(NEXUS_DspVideoEncoderHandle encoder, BAVC_EncodePictureBuffer *picture);
#else
NEXUS_Error NEXUS_DspVideoEncoder_EnqueuePicture_isr(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderPicture *picture);
NEXUS_Error NEXUS_DspVideoEncoder_DequeuePicture_isr(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderPicture *picture);
NEXUS_Error NEXUS_DspVideoEncoder_GetExtInterruptInfo_priv(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderExtInterruptInfo *extIntInfo);
#endif
NEXUS_Error NEXUS_DspVideoEncoder_GetDelayRange_priv(NEXUS_DspVideoEncoderHandle encoder, uint32_t width, uint32_t height, NEXUS_VideoFrameRate framerate, uint32_t bitrate, uint32_t *delay);
void NEXUS_DspVideoEncoder_GetUserDataSettings_priv(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderUserDataSettings *userDataSettings);
NEXUS_Error NEXUS_DspVideoEncoder_GetSettings_priv(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderSettings *pSettings);
NEXUS_Error NEXUS_DspVideoEncoder_SetSettings_priv(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderSettings *pSettings);
void NEXUS_DspVideoEncoder_P_Watchdog_priv(NEXUS_DspVideoEncoderHandle encoder);
void NEXUS_DspVideoEncoder_SetWatchdogCallback_priv(NEXUS_CallbackDesc *watchdog);
void NEXUS_DspVideoEncoder_Watchdog_priv(void);
#endif /* NEXUS_AUDIO_DSP_VIDEO_ENCODER_H__ */

