/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#ifndef NEXUS_VIDEO_DECODER_EXTRA_H__
#define NEXUS_VIDEO_DECODER_EXTRA_H__

#include "nexus_video_decoder.h"
#include "nexus_striped_surface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Reset the decoder

Description:
This will cause a reload of video decoder firmware. All current decodes will be restarted and all Nexus handles remain valid.

This could cause the reset of more than just this NEXUS_VideoDecoderHandle, depending on internal implementation requirements.

This is provided for application API requirements. There should be no need for any application to call this function.
If you find that you must call this function, please notify Broadcom so that we can address the problem.
**/
void NEXUS_VideoDecoder_Reset(
    NEXUS_VideoDecoderHandle handle
    );

/**
Summary:
Extended status information from internal "data ready" callback.

Description:
This status information has internal usage and is less likely to be used in typical applications. However, for completeness we have provided it.
Because of its internal nature, this structure is subject to change. Some fields may only apply for certain codecs.
Fields may be added or removed for internal reasons.

See NEXUS_VideoDecoderStatus for the most typical status information.
See NEXUS_VideoDecoderStreamInformation for additional information regarding the last decoded picture (at the head of the picture queue).

See magnum/commonutils/CHIP/bavc.h for additional documentation on these fields
**/
typedef struct NEXUS_VideoDecoderExtendedStatus
{
    unsigned                 dataReadyCount; /* # of dataReady interrupts that have fired since Start.
                                                For a 60fps stream, this number will wrap in 2 years. */
    NEXUS_PicturePolarity    interruptPolarity;
    NEXUS_PicturePolarity    sourcePolarity;
    bool                     isMpeg1; /* if the current NEXUS_VideoCodec is MPEG, this reports true if the VideoDecoder detects this is MPEG1, not MPEG2. */
    bool                     isYCbCr422; /* if true, the decoded video provided to Display if 4:2:2, otherwise it is 4:2:0 */
    bool                     fieldChrominanceInterpolationMode; /* true for field, false for frame */
    uint32_t                 adjQp;
    bool                     pictureRepeatFlag;
    uint32_t                 luminanceNMBY;
    uint32_t                 chrominanceNMBY;
    unsigned                 stripeWidth;
    bool                     lastPictureFlag;   /* Set to true once a BTP FLUSH/LAST/FLUSH sequence was encountered.
                                                   This will also trigger the sourceChanged callback when
                                                   transitioning from false to true. This flag is reset
                                                   upon the next call to NEXUS_VideoDecoder_Stop(). */
    bool                     captureCrc; /* only true if the decoder is set in CRC mode. */
    uint32_t                 idrPicID; /* only valid if captureCrc is true */
    int32_t                  picOrderCnt; /* only valid if captureCrc is true */
    uint32_t                 sourceClipTop;
    uint32_t                 sourceClipLeft;
    uint32_t                 lumaRangeRemapping;
    uint32_t                 chromaRangeRemapping;
    struct {
        int mainIndex;
        int mainSwRaveIndex; /* if -1, then unused */
        int enhancementIndex; /* if -1, then unused */
        int enhancementSwRaveIndex; /* if -1, then unused */
    } rave;
    struct {
        unsigned firmware;
    } version;

    int pidChannelIndex;  /* The source of the video data from Transport. -1 indicates no channel (decoder is stopped) */
    int stcChannelIndex;  /* The StcChannel that connects audio and video decode to provide lipsync. -1 indicated no STC channel (decoder is stopped) */
} NEXUS_VideoDecoderExtendedStatus;

/**
Summary:
Get NEXUS_VideoDecoderExtendedStatus
**/
NEXUS_Error NEXUS_VideoDecoder_GetExtendedStatus(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderExtendedStatus *pStatus /* [out] */
    );

/**
Summary:
Get extended settings
**/
void NEXUS_VideoDecoder_GetExtendedSettings(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderExtendedSettings *pSettings /* [out] */
    );

/**
Summary:
Set extended settings
**/
NEXUS_Error NEXUS_VideoDecoder_SetExtendedSettings(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderExtendedSettings *pSettings
    );

/**
Summary:
Get NEXUS_VideoDecoder3DTVStatus

Description:
The status is cleared each time before starting decode, and is set when the first 3DTV message is found.
**/
NEXUS_Error NEXUS_VideoDecoder_Get3DTVStatus(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoder3DTVStatus *pStatus /* [out] */
    );

/**
Summary:
Create a striped surface which maps to the most recent picture returned from the decoder

Description:
Returns NULL if CreateStripedSurface was already called for the current picture.

See NEXUS_VideoDecoder_GetDecodedFrames for a more feature-complete alternative;
CreateStripedSurface does not return PTS or other metadata and lacks a FIFO interface.

This is used for a AVD->M2MC->GFD path (aka video-as-graphics) for decoded video.
See NEXUS_Display_DriveVideoDecoder.
**/
NEXUS_StripedSurfaceHandle NEXUS_VideoDecoder_CreateStripedSurface(
    NEXUS_VideoDecoderHandle handle
    );

/**
Summary:
Destroy the surface returned by NEXUS_VideoDecoder_CreateStripedSurface
**/
void NEXUS_VideoDecoder_DestroyStripedSurface(
    NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_StripedSurfaceHandle stripedSurface
    );

/**
Summary:
Create a striped surface which maps to the most recent set of mosaic picture returned from the decoder

Description:
See NEXUS_VideoDecoder_CreateStripedSurface for more information.
**/
NEXUS_Error NEXUS_VideoDecoder_CreateStripedMosaicSurfaces(
    NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_StripedSurfaceHandle *pStripedSurfaces, /* [out] attr{nelem=maxSurfaces;nelem_out=pSurfaceCount;reserved=10} array of striped surfaces */
    unsigned int maxSurfaces, /* total number of elements in pStripedSurfaces array */
    unsigned int *pSurfaceCount  /* [out] number of surfaces returned in pStripedSurfaces */
    );

/**
Summary:
Destroy the surfaces returned by NEXUS_VideoDecoder_CreateStripedMosaicSurfaces
**/
void NEXUS_VideoDecoder_DestroyStripedMosaicSurfaces( /* attr{local=true} */
    NEXUS_VideoDecoderHandle videoDecoder,
    const NEXUS_StripedSurfaceHandle *pStripedSurfaces, /* attr{nelem=surfaceCount;reserved=10} array of striped surfaces */
    unsigned int surfaceCount
    );

/**
Summary:
DEPRECATED: use NEXUS_VideoDecoderStatus.bufferLatency instead
Get the PTS for the picture most recently added to the VideoDecoder's FIFO (i.e. CDB)

Description:
The difference between this PTS and the decoder's current PTS (NEXUS_VideoDecoderStatus.pts) represents
the amount of time in the decoder's FIFO. The app is responsible to take into account possible PTS discontinuities.

This function will fail if the FIFO is empty and there are no PTS's. This can happen if the FIFO has run empty.
**/
NEXUS_Error NEXUS_VideoDecoder_GetMostRecentPts(
    NEXUS_VideoDecoderHandle handle,
    uint32_t *pPts /* [out] */
    );

/**
Summary:
Enable the capture of AVD outer ARC messages
**/
NEXUS_Error NEXUS_VideoDecoderModule_SetDebugLog(
    unsigned avdCore,
    bool enabled
    );

/**
Summary:
Capture AVD outer ARC messages after NEXUS_VideoDecoder_SetDebugLog enables the feature
**/
NEXUS_Error NEXUS_VideoDecoderModule_ReadDebugLog(
    unsigned avdCore,
    void *buffer,         /* [out] attr{nelem=bufferSize;nelem_out=pAmountRead} */
    unsigned bufferSize,
    unsigned *pAmountRead /* [out] */
    );

/**
Summary:
Send debug command to AVD. This may cause a watchdog or large flow of data to ReadDebugLog. Use with care.

Example code:
NEXUS_VideoDecoderModuleDebugCommand cmd = {"help"};
NEXUS_VideoDecoderModule_SendDebugCommand(0, &cmd);

**/
typedef struct NEXUS_VideoDecoderModuleDebugCommand
{
    char command[128]; /* must be null terminated */
} NEXUS_VideoDecoderModuleDebugCommand;

NEXUS_Error NEXUS_VideoDecoderModule_SendDebugCommand(
    unsigned avdCore,
    const NEXUS_VideoDecoderModuleDebugCommand *pCommand
    );

/**
Summary:
Read AVD CRC

Description:
To enable AVD CRC capture, set NEXUS_VideoDecoderExtendedSettings.crcFifoSize to a non-zero value.
**/
NEXUS_Error NEXUS_VideoDecoder_GetCrcData(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderCrc *pEntries, /* attr{nelem=numEntries;nelem_out=pNumReturned} */
    unsigned numEntries,
    unsigned *pNumReturned
    );

/**
Summary:
Get decoder fifo status
**/
NEXUS_Error NEXUS_VideoDecoder_GetFifoStatus(
    NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_VideoDecoderFifoStatus *pStatus /* [out] */
    );

/**
Summary:
Print currently opened decoders
**/
void NEXUS_VideoDecoderModule_PrintDecoders(void);

#ifdef __cplusplus
}
#endif

#endif
