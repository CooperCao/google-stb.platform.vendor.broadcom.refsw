/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
 **************************************************************************/
#ifndef NEXUS_STILL_DECODER_H__
#define NEXUS_STILL_DECODER_H__

#include "nexus_video_decoder.h"
#include "nexus_striped_surface.h"

/*=**************************
The StillDecoder is an extension on VideoDecoder. It provides a method for decoding a single still frame
into memory for use as a graphics surface.
****************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for Still Decoder
**/
typedef struct NEXUS_StillDecoder *NEXUS_StillDecoderHandle;

/**
Summary:
Settings used to open a still decoder.
**/
typedef struct NEXUS_StillDecoderOpenSettings
{
    unsigned fifoSize;
    NEXUS_HeapHandle cdbHeap; /* Heap for CDB. For compressed restricted region (CRR), set to heap[NEXUS_VIDEO_SECURE_HEAP]. */
    NEXUS_HeapHandle heap; /* optional picture buffer heap. This overrides the NEXUS_VideoDecoderModuleInternalSettings.avdHeapIndex setting. */
    
    bool supportedCodecs[NEXUS_VideoCodec_eMax]; /* list of codecs that can be decoded. */
    unsigned maxWidth, maxHeight; /* max resolution of still */
} NEXUS_StillDecoderOpenSettings;

/**
Summary:
Get default settings for use in NEXUS_StillDecoder_Open.
**/
void NEXUS_StillDecoder_GetDefaultOpenSettings(
    NEXUS_StillDecoderOpenSettings *pSettings
    );

/**
Summary:
Open an interface to decode I-frame still pictures.
**/
NEXUS_StillDecoderHandle NEXUS_StillDecoder_Open(  /* attr{destructor=NEXUS_StillDecoder_Close}  */
    NEXUS_VideoDecoderHandle deprecated, /* attr{null_allowed=y} deprecated and not needed. we recommend setting NULL and not creating
                                            a false dependency on a VideoDecoder handle. */
    unsigned index,
    const NEXUS_StillDecoderOpenSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Close the still decoder.
**/
void NEXUS_StillDecoder_Close(
    NEXUS_StillDecoderHandle handle
    );

/**
Summary:
Settings used to decode a still.
**/
typedef struct NEXUS_StillDecoderStartSettings
{
    NEXUS_PidChannelHandle pidChannel;    /* Transport source of still. */
    NEXUS_VideoCodec codec;               /* Codec of the still. */
    NEXUS_CallbackDesc endCodeFound;   /* Callback when StillDecoder find the end code and begins actual decoding of the still */
    NEXUS_CallbackDesc stillPictureReady; /* Callback when the still picture has been decoded and the surface can be retrieved with NEXUS_StillDecoder_GetSurface */
    
    struct {
        unsigned maxWidth, maxHeight;
        NEXUS_MemoryBlockHandle memory; /* optional buffer */
        unsigned memoryOffset;
        void *buffer; /* attr{memory=cached} deprecated. optional buffer pointer. only used if 'memory' is NULL. */
        unsigned size; /* size of memory pointed to by buffer */    
    } output;
} NEXUS_StillDecoderStartSettings;

/**
Summary:
Get default settings for use in NEXUS_StillDecoder_Configure.
**/
void NEXUS_StillDecoder_GetDefaultStartSettings(
    NEXUS_StillDecoderStartSettings *pSettings
    );

/**
Summary:
Start decoding a still

Description:
After calling Start, you should feed into into the pid channel. The StillDecoder will begin to monitor
the pid channel and wait until the end code is found. When it is found you will receive an endCodeFound callback.
Then, it will begin decoding the still. When the still is available you will receieve a stillDecodeReady callback.

It's not required that you receive these callbacks. You can also poll the StillDecoder and wait for NEXUS_StillDecoderStatus.stillPictureReady
to become true.

When your still is ready, call NEXUS_StillDecoder_GetStripedSurface to get the result. See that function for notes about
how to use the resulting surface.

If the still does not decode (for example, an invalid picture), your application should wait for its own desired timeout, then call NEXUS_StillDecoder_Stop.
The Stop is required to reset state and allow for the next Start.
**/
NEXUS_Error NEXUS_StillDecoder_Start(
    NEXUS_StillDecoderHandle handle,
    const NEXUS_StillDecoderStartSettings *pSettings
    );

/**
Summary:
Stop decoding a still

Description:
You must call Stop before Starting another still decode.

If your still has been decoded, calling NEXUS_StillDecoder_Stop will not erase the results. You can still call
NEXUS_StillDecoder_GetStripedSurface to retrieve the results.
**/
void NEXUS_StillDecoder_Stop(
    NEXUS_StillDecoderHandle handle
    );

/**
Summary:
Status of still picture decode
**/
typedef struct NEXUS_StillDecoderStatus
{
    bool endCodeFound;          /* true if StillDecoder has detected an end code (0xB7 for MPEG, 0x0A for AVC) after NEXUS_StillDecoder_Start was called */
    bool stillPictureReady;     /* true if StillDecoder has successfully decoded a still after NEXUS_StillDecoder_Start was called */
    unsigned avdStatusBlock;    /* snap shot of the AVD channel status */
    struct {
        int mainIndex;
        int mainSwRaveIndex; /* if -1, then unused */
    } rave;    
} NEXUS_StillDecoderStatus;

/**
Summary:
Get status
**/
NEXUS_Error NEXUS_StillDecoder_GetStatus(
    NEXUS_StillDecoderHandle handle,
    NEXUS_StillDecoderStatus *pStatus /* [out] */
    );

/**
Summary:
Get the results of the decoded still picture.

Description:
Call NEXUS_Grahics2D_Destripe to obtain a NEXUS_SurfaceHandle from this information.
If stillPictureReady has not been marked true since that last NEXUS_StillDecoder_Start, this function will fail.

Do not call NEXUS_StripedSurface_Destroy on this handle. Instead, NEXUS_StillDecoder_ReleaseStripedSurface when you are done.
The surface will be implicitly released on NEXUS_StillDecoder_Close or the next NEXUS_StillDecoder_Start call.
**/
NEXUS_Error NEXUS_StillDecoder_GetStripedSurface(
    NEXUS_StillDecoderHandle handle,
    NEXUS_StripedSurfaceHandle *pStripedSurface /* [out] */
    );

/**
Release striped surface obtained with NEXUS_StillDecoder_GetStripedSurface.
The striped surface is no longer usable after this.
**/
void NEXUS_StillDecoder_ReleaseStripedSurface(
    NEXUS_StillDecoderHandle handle,
    NEXUS_StripedSurfaceHandle stripedSurface
    );

/**
Summary:
Param for NEXUS_StillDecoder_RequestMemorySize
**/
typedef struct NEXUS_StillDecoderMemoryRequest {
    unsigned width;
    unsigned height;
    NEXUS_VideoCodec codec;
} NEXUS_StillDecoderMemoryRequest;

/**
Summary:
**/
void NEXUS_StillDecoder_GetDefaultMemoryRequest(
    NEXUS_StillDecoderMemoryRequest *pRequest /* [out] */
    );

/**
Summary:
Learn the heap size required to decode a still. This includes space for picture data and decoder meta-data.
**/
NEXUS_Error NEXUS_StillDecoder_RequestMemorySize(
    NEXUS_StillDecoderHandle handle,
    const NEXUS_StillDecoderMemoryRequest *pRequest,
    unsigned *pSize /* [out] minimum size required. copy this value to NEXUS_StillDecoderStartSettings.output.size. */
    );
    
#ifdef __cplusplus
}
#endif

#endif
