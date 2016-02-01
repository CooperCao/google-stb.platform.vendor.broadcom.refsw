/***************************************************************************
 *     (c)2010-2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef NEXUS_SIMPLE_VIDEO_DECODER_H__
#define NEXUS_SIMPLE_VIDEO_DECODER_H__

#include "nexus_types.h"
#include "nexus_video_decoder_types.h"
#include "nexus_simple_stc_channel.h"
#include "nexus_display_types.h"
#include "nexus_simple_decoder_types.h"
#include "nexus_pid_channel.h"
#include "nexus_surface.h"
#include "nexus_striped_surface.h"
#include "nexus_picture_quality_types.h"
#include "nexus_video_image_input.h"
#include "nexus_video_decoder_extra.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
SimpleVideoDecoder provides a self-configuring video decoder for untrusted clients.
It is used in multi-process systems where garbage collection is required.
It is able to dynamically allocate memory for start-time max width/height.

HD/SD simul and main/pip configurations are supported. Dual output configurations (i.e. TV1 and TV2) are currently not supported.
**/

/**
Summary:
**/
typedef struct NEXUS_SimpleVideoDecoder *NEXUS_SimpleVideoDecoderHandle;

/**
Summary:
**/
NEXUS_SimpleVideoDecoderHandle NEXUS_SimpleVideoDecoder_Acquire( /* attr{release=NEXUS_SimpleVideoDecoder_Release}  */
    unsigned index
    );

/**
Summary:
Stops decode and releases the handle. The handle can no longer be used.
**/
void NEXUS_SimpleVideoDecoder_Release(
    NEXUS_SimpleVideoDecoderHandle handle
    );

/**
Summary:
**/
#define NEXUS_SIMPLE_DECODER_MAX_SURFACES 3
typedef struct NEXUS_SimpleVideoDecoderStartSettings
{
    NEXUS_VideoDecoderStartSettings settings;
    unsigned maxWidth, maxHeight; /* max source size dimensions that will be decoded in this session.
                                     set to 0,0 if you want to use defaults. */

    bool displayEnabled; /* If set to false nexus will not route video to the display */
    bool smoothResolutionChange; /* Prevent any BVN reconfig black frames due to source resolution or scaling changes.
                                    This may exceed BVN RTS bandwidth on non-box mode systems.
                                    Box mode systems support this feature without this boolean being set; the only additional effect is disabling scaleFactorRounding. */
    bool lowDelayImageInput;    /* If true, when ImageInput is used it will operate in a low-delay mode
                                   where pictures will be delivered to the display with minimal delay
                                   bypassing TSM and frame rate conversion logic.  Set to false if you
                                   want to enable TSM/StcChannel support for ImageInput. */
} NEXUS_SimpleVideoDecoderStartSettings;

/**
Summary:
Set or clear SimpleStcChannel for lipsync.

Description:
If you set and start audio and video separately, you will get a glitch when both audio and video have started
and SyncChannel makes an adjustment. Another option is to setNEXUS_SimpleStcChannelSettings.sync = false and
get only basic TSM, off by up to 100 msec, with no glitch.
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_SetStcChannel(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_SimpleStcChannelHandle stcChannel /* attr{null_allowed=y} use NULL to clear. */
    );

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(
    NEXUS_SimpleVideoDecoderStartSettings *pSettings /* [out] */
    );

/**
Summary:
Start decode

Description:
Internally, this will set up the connection to the display, allocate memory and start decode.
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_Start(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_SimpleVideoDecoderStartSettings *pSettings
    );

/**
Summary:
Stop decode, but don't free memory.

Description:
This allows the last picture to remain visible. Useful for channel change.
**/
void NEXUS_SimpleVideoDecoder_Stop(
    NEXUS_SimpleVideoDecoderHandle handle
    );

/**
Summary:
Stop decode and free all memory.

Description:
The last picture is no longer available.
**/
void NEXUS_SimpleVideoDecoder_StopAndFree(
    NEXUS_SimpleVideoDecoderHandle handle
    );

typedef struct NEXUS_SimpleVideoDecoderStartCaptureSettings
{
    bool displayEnabled; /* override NEXUS_SimpleVideoDecoderStartSettings.displayEnabled while capture is started */
    NEXUS_SurfaceHandle surface[NEXUS_SIMPLE_DECODER_MAX_SURFACES]; /* TODO: array of handles not verified yet */
    bool forceFrameDestripe; /* Destripe pictures from VideoDecoder as frames to improve quality.
                                If top/bottom are temporily different, app can chose to use only one field of
                                content to avoid jaggies.
                                If not set, only top field may be destriped. */
} NEXUS_SimpleVideoDecoderStartCaptureSettings;

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(
    NEXUS_SimpleVideoDecoderStartCaptureSettings *pSettings /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_StartCapture(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_SimpleVideoDecoderStartCaptureSettings *pSettings
    );

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_StopCapture(
    NEXUS_SimpleVideoDecoderHandle handle
    );

/**
Summary:
**/
typedef struct NEXUS_VideoDecoderFrameStatus NEXUS_SimpleVideoDecoderCaptureStatus;

/**
Summary:
Request captured surfaces and their status structs
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_SurfaceHandle *pSurface, /* attr{nelem=numEntries;nelem_out=pNumReturned} array of surfaces */
    NEXUS_SimpleVideoDecoderCaptureStatus *pStatus, /* attr{nelem=numEntries;nelem_out=pNumReturned;null_allowed=y} array of status structs corresponding to surfaces */
    unsigned numEntries,   /* applies to both pSurface and pStatus arrays */
    unsigned *pNumReturned /* applies to both pSurface and pStatus arrays */
    );

/**
Summary:
Recycle captured surfaces obtained from NEXUS_SimpleVideoDecoder_GetCapturedSurfaces
**/
void NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_SurfaceHandle *pSurface, /* attr{nelem=numEntries} */
    unsigned numEntries
    );

/**
Summary:
Get a decoded frame

Description:
If NEXUS_VideoDecoderStartSettings.appDisplayManagement is true,
this call will return the next decoded frame to the application.
It must be returned with
NEXUS_SimpleVideoDecoder_ReturnDecodedFrames.
*/
NEXUS_Error NEXUS_SimpleVideoDecoder_GetDecodedFrames(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoDecoderFrameStatus *pStatus,  /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned} [out] */
    unsigned numEntries,
    unsigned *pNumEntriesReturned /* [out] */
    );

/**
Summary:
Get Default Settings for a returned frame
*/
void NEXUS_SimpleVideoDecoder_GetDefaultReturnFrameSettings(
    NEXUS_VideoDecoderReturnFrameSettings *pSettings    /* [out] */
    );

/**
Summary:
Return a decoded frame

Description:
If NEXUS_VideoDecoderStartSettings.appDisplayManagement is true,
this call will display or drop a frame returned from
NEXUS_SimpleVideoDecoder_GetDecodedFrames.  Once returned, the
frame can not be reused and will become invalid
*/
NEXUS_Error NEXUS_SimpleVideoDecoder_ReturnDecodedFrames(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_VideoDecoderReturnFrameSettings *pSettings, /* attr{null_allowed=y;nelem=numFrames} Settings for each returned frame.  Pass NULL for defaults. */
    unsigned numFrames                                      /* Number of frames to return to the decoder */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_GetStatus(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoDecoderStatus *pStatus   /* [out] Note that the regular NEXUS_VideoDecoder structure is used */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_GetExtendedStatus(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoDecoderExtendedStatus *pStatus   /* [out] Note that the regular NEXUS_VideoDecoder structure is used */
    );

/**
Summary:
Check NEXUS_VideoDecoderStreamInformation.valid if returned status is valid.
**/
void NEXUS_SimpleVideoDecoder_GetStreamInformation(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoDecoderStreamInformation *pStreamInfo
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_Get3DTVStatus(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoDecoder3DTVStatus *pStatus
    );

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_Flush(
    NEXUS_SimpleVideoDecoderHandle handle
    );

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_GetTrickState(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoDecoderTrickState *pSettings   /* [out] Note that the regular NEXUS_VideoDecoder structure is used */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_SetTrickState(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_VideoDecoderTrickState *pSettings   /* Note that the regular NEXUS_VideoDecoder structure is used */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_FrameAdvance(
    NEXUS_SimpleVideoDecoderHandle handle
    );

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_GetPlaybackSettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoDecoderPlaybackSettings *pSettings /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_SetPlaybackSettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_VideoDecoderPlaybackSettings *pSettings
    );

typedef struct NEXUS_SimpleVideoDecoderClientSettings
{
    NEXUS_CallbackDesc resourceChanged; /* underlying video decoder was added or removed. check NEXUS_SimpleVideoDecoderClientStatus.enabled. */
    NEXUS_VideoWindowAfdSettings afdSettings;
    bool closedCaptionRouting; /* parse userdata for closed captioning and route to displays capable of VBI encoding */
} NEXUS_SimpleVideoDecoderClientSettings;

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_GetClientSettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_SimpleVideoDecoderClientSettings *pSettings
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_SetClientSettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_SimpleVideoDecoderClientSettings *pSettings
    );

/**
Summary:
Status that isn't passthrough from VideoDecoder
**/
typedef struct NEXUS_SimpleVideoDecoderClientStatus
{
    bool enabled; /* set true if the underlying video decoder is available */
    unsigned numDroppedCaptureSurfaces; /* due to overflow */
    uint32_t afdValue; /* See NEXUS_VideoDecoderUserDataStatus.afdValue for usage.
                          If this value changes, NEXUS_VideoDecoderSettings.afdChanged will be fired.*/
} NEXUS_SimpleVideoDecoderClientStatus;

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_GetClientStatus(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_SimpleVideoDecoderClientStatus *pStatus
    );

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_GetSettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoDecoderSettings *pSettings
    );

/**
Summary:
Passthrough video decoder settings.

Description:
However, these settings can be modified by NEXUS_SimpleVideoDecoder_Start, so the app should not cache them.
For example, Start will set codec, maxWidth and maxHeight based on its start params.
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_SetSettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_VideoDecoderSettings *pSettings
    );

/**
Summary:
**/
void NEXUS_SimpleVideoDecoder_GetExtendedSettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoDecoderExtendedSettings *pSettings
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_SetExtendedSettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_VideoDecoderExtendedSettings *pSettings
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_SetStartPts(
    NEXUS_SimpleVideoDecoderHandle handle,
    uint32_t pts
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_GetNextPts(
    NEXUS_SimpleVideoDecoderHandle handle,
    uint32_t *pPts
    );

/**
Summary:
Capture raw user data

Description:
Data is guaranteed to be returned in whole blocks. That is, NEXUS_UserDataHeader plus payload.
Zero, one or more blocks may be return in each call.

There are several differences between SimpleVideoDecoder and VideoDecoder's handling of user data.
1) The regular VideoDecoder returns raw user data, but requires the caller to have cpu access to
video memory. SimpleVideoDecoder makes a copy into the caller's memory so that video memory is protected.
2) The regular VideoDecoder internally parses userdata and routes the resulting EIA 608/708 CC data to
the display module for display and caller access. This requires the video decoder module to filter different
types of user data. SimpleVideoDecoder requires the client to parse the userdata. This allows the client
to filter as needed. No server-side filter is required.

See nexus/nxclient/apps/ccgfx.c and related to code for SimpleVideoDecoder userdata capture and client-side parsing
and CC graphics rendering.
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_ReadUserDataBuffer(
    NEXUS_SimpleVideoDecoderHandle handle,
    void *pBuffer,      /* [out] attr{nelem=bufferSize;nelem_out=pBytesCopied;reserved=1024} pointer for userdata buffer */
    size_t bufferSize,  /* number of bytes of userdata that can be copied */
    size_t *pBytesCopied /* [out] number of bytes of userdata that was copied */
    );

/**
Summary:
Flush user data
**/
void NEXUS_SimpleVideoDecoder_FlushUserData(
    NEXUS_SimpleVideoDecoderHandle videoDecoder
    );

/**
deprecated
**/
void NEXUS_SimpleVideoDecoder_SetCacheEnabled(
    NEXUS_SimpleVideoDecoderHandle handle,
    bool enabled
    );

/**
Summary:
Get FIFO Status
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_GetFifoStatus(
    NEXUS_SimpleVideoDecoderHandle videoDecoder,
    NEXUS_VideoDecoderFifoStatus *pStatus /* [out] */
    );

/**
Summary:
PQ settings that apply per window, not per display
**/
typedef struct NEXUS_SimpleVideoDecoderPictureQualitySettings
{
    NEXUS_PictureCtrlCommonSettings common;
    NEXUS_VideoWindowDnrSettings dnr;
    NEXUS_VideoWindowAnrSettings anr;
    NEXUS_VideoWindowMadSettings mad;
    NEXUS_VideoWindowScalerSettings scaler;
} NEXUS_SimpleVideoDecoderPictureQualitySettings;

void NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_SimpleVideoDecoderPictureQualitySettings *pSettings
    );

NEXUS_Error NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_SimpleVideoDecoderPictureQualitySettings *pSettings
    );

/**
Summary:
Request interface for "graphics as video"
**/
NEXUS_VideoImageInputHandle NEXUS_SimpleVideoDecoder_StartImageInput(
    NEXUS_SimpleVideoDecoderHandle handle,
    const NEXUS_SimpleVideoDecoderStartSettings *pStartSettings /* attr{null_allowed=y} */
    );

void NEXUS_SimpleVideoDecoder_StopImageInput(
    NEXUS_SimpleVideoDecoderHandle handle
    );

/**
Summary:
Connect HdmiInput interface
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_StartHdmiInput(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_HdmiInputHandle hdmiInput,
    const NEXUS_SimpleVideoDecoderStartSettings *pStartSettings /* attr{null_allowed=y} */
    );

void NEXUS_SimpleVideoDecoder_StopHdmiInput(
    NEXUS_SimpleVideoDecoderHandle handle
    );

/**
Summary:
Connect HdDvi interface
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_StartHdDviInput(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_HdDviInputHandle hdDviInput,
    const NEXUS_SimpleVideoDecoderStartSettings *pStartSettings /* attr{null_allowed=y} */
    );

void NEXUS_SimpleVideoDecoder_StopHdDviInput(
    NEXUS_SimpleVideoDecoderHandle handle
    );

/**
Summary:
Read AVD CRC

Description:
Will set NEXUS_VideoDecoderExtendedSettings.crcFifoSize if zero.
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_GetCrcData(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoDecoderCrc *pEntries, /* attr{nelem=numEntries;nelem_out=pNumReturned} */
    unsigned numEntries,
    unsigned *pNumReturned
    );

/**
Summary:
Get MFD CRC data

Description:
Will set NEXUS_VideoInputSettings.crcQueueSize if zero.
**/
NEXUS_Error NEXUS_SimpleVideoDecoder_GetVideoInputCrcData(
    NEXUS_SimpleVideoDecoderHandle handle,
    NEXUS_VideoInputCrcData *pEntries, /* attr{nelem=numEntries;nelem_out=pNumReturned} array of crc data structures */
    unsigned numEntries,
    unsigned *pNumReturned
    );

NEXUS_Error NEXUS_SimpleVideoDecoder_SetSdOverride(
    NEXUS_SimpleVideoDecoderHandle handle,
    bool enabled
    );

#ifdef __cplusplus
}
#endif

#endif
