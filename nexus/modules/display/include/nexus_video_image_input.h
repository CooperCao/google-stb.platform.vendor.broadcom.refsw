/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_VIDEO_IMAGE_INPUT_H__
#define NEXUS_VIDEO_IMAGE_INPUT_H__

#include "nexus_types.h"
#include "nexus_display_types.h"
#include "nexus_surface.h"
#include "nexus_striped_surface.h"
#if NEXUS_HAS_TRANSPORT
#include "nexus_stc_channel.h"
#else
typedef void *NEXUS_StcChannelHandle;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*=*
The VideoImageInput interface provides a way to feed graphics into the system as a video source.
The VideoImageInput takes a surface and converts in into a VideoInput. Then this VideoInput can
be connected to a VideoWindow and the graphics will be displayed on the screen.

In the current implementation, VideoImageInput shares hardware resources with the VideoDecoder module (the MFD).
As a consequence, the user shall carefully choose the index parameter that is passed into NEXUS_VideoImageInput_Open
function. As a rule, the output of a VideoDecoder and a VideoImageInput with a matching 'index' can't be rendered
simultaneously. And before switching video between VideoDecoder and VideoImageInput, the user must call
NEXUS_VideoInput_Shutdown on unused instance of VideoInput.
**/

#define NEXUS_IMAGEINPUT_BUSY       NEXUS_MAKE_ERR_CODE(0x10A, 1)  /* image is in use and can't be disturbed */
#define NEXUS_IMAGEINPUT_QUEUE_FULL NEXUS_MAKE_ERR_CODE(0x10A, 2)  /* queue is full, can't add more          */

/**
Summary:
 Entry point of graphics into the BVN
**/
typedef enum NEXUS_VideoImageInputType
{
    NEXUS_VideoImageInput_eMfd,
    NEXUS_VideoImageInput_eVfd,
    NEXUS_VideoImageInput_eMax
} NEXUS_VideoImageInputType;
/**
Summary:
Handle for the VideoImageInput interface
**/
typedef struct NEXUS_VideoImageInput *NEXUS_VideoImageInputHandle;

/**
Summary:
Settings for VideoImageInput input
**/
typedef struct NEXUS_VideoImageInputSettings {
    /* These settings can only be set at open time: */
    NEXUS_VideoImageInputType type;

    NEXUS_CallbackDesc imageCallback;  /* Callback called when image has been released from for display, NULL or a new image must be passed in for this to happen. */
    unsigned qScale; /* Q-Scale factor used in DNR */
    bool duplicate;  /* should be set to true if next frame is indentical to the previous frame */
    NEXUS_HeapHandle heap; /* If non-NULL, the video path will allocate buffers from this heap based on NEXUS_DisplayHeapSettings.
                              If NULL, the video path will allocate buffers from the Display Module heap based on NEXUS_DisplayModuleSettings. */
    bool progressiveScan1080; /* if true, 1920x1080 stills will be scanned out as a single progressive frame.
                              you should only set this if you know that your MFD HW and RTS allows for it. for live decode, this is 1080p60 capability.
                              if false, 1920x1080 stills will be scanned out as two interlaced fields. */
    unsigned fifoSize;        /* size of image Input queue */

    bool lowDelayMode;        /* If true (default), pictures will be delivered to the display with minimal delay, bypassing TSM and frame rate conversion logic.
                                 Can only be changed when picture queue is empty.  Only available for MFD input */
    NEXUS_StcChannelHandle stcChannel;  /* Can only be changed when picture queue is empty.  Requires lowDelayMode=false.
                                           Supplying a non-NULL value for this during an Open or SetSettings call will be
                                           treated as the equivalent of a Start call, which will cause a reset of the STC
                                           in NRT mode. Setting this member to NULL via a SetSettings call will be treated
                                           as the equivalent of a Stop call. It is assumed that all configuration of both
                                           StcChannel and ImageInput required to start evaluating image time stamps has
                                           been completed before the stcChannel is provided to the ImageInput. See the
                                           various expectations of NEXUS_VideoDecoder_Start and _Stop for examples of
                                           what is expected by ImageInput once a non-NULL stcChannel is set. */
    bool tsmEnabled;                    /* Set to false to disable TSM if required (e.g. trick modes).  Ignored if lowDelayMode=true or stcChannel=NULL. */

    NEXUS_CallbackDesc ptsError;         /* Fires on any PTS discontinuity. This callback is used for application notification. No response is required for TSM. */

    NEXUS_CallbackDesc firstPtsPassed;   /* Fires when the first picture is delivered to the display after TSM pass.
                                            This callback is used for application notification. No response is required for TSM. */

    bool secureVideo; /* route through secure BVN */
} NEXUS_VideoImageInputSettings;

/**
Summary:
Settings for VideoImageInputSurfaceSettings
**/
typedef struct NEXUS_VideoImageInputSurfaceSettings {
    unsigned displayVsyncs;     /* Number of times this picture must be displayed before next picture in queue is displayed
                                   Only used if NEXUS_VideoImageInputSettings.lowDelayMode = true */
    bool     infront;           /* This picture will be the next picture displayed. Any pictures already queued will be purged. Default is false
                                   Only used if NEXUS_VideoImageInputSettings.lowDelayMode = true */

    uint32_t pts;               /* PTS value, used only if NEXUS_VideoImageInputSettings.lowDelayMode=false. Default is ZERO.
                                   NOTE: true pts's are 33bit, so this is a rounded down 32 bit pts */
    bool ptsValid;              /* If true, the PTS value above is valid. */

    NEXUS_VideoFrameRate frameRate;      /* User-specified frame rate. For some streams, the decoder cannot determine the frame rate.
                                            The user may be able to determine the frame rate from meta data.
                                            This value is overridden by any value found in the stream. */
    NEXUS_AspectRatio aspectRatio;       /* User-specified aspect ratio. For some streams, the decoder cannot determine the aspect ratio.
                                            The user may be able to determine the aspect ratio from meta data.
                                            This value is overridden by any value found in the stream. */
    struct {
        unsigned x, y;
    } sampleAspectRatio;                 /* Valid is user-specified aspectRatio is NEXUS_AspectRatio_eSar */

    NEXUS_PictureCoding pictureCoding;  /* Type of picture (I/P/B) */

    NEXUS_PictureScan sourceFormat;
    NEXUS_PictureScan sequence;

    bool endOfStream;                   /* If true, all other parameters are ignored and an end-of-stream
                                           notification is sent downstream to the video subsystem.  This
                                           is useful when encoding the output to cause the encoder to finalize
                                           the bitstream.  */

    NEXUS_PicturePullDown pullDown;

    NEXUS_StripedSurfaceHandle stripedSurface; /* if not NULL and surface is NULL, push the stripedSurface;
                                  If the surface and stripedSurface are both NULL, it flushes the surface queue or send EOS
                                  depending on NEXUS_VideoImageInputSurfaceSettings.endOfStream. When striped surface and regular
                                  surface are mixed in the push queue, the NEXUS_VideoImageInput_RecycleSurface may recycle mixed
                                  stripedSurface and regular surface; and user is responsible to check the handle value. */
} NEXUS_VideoImageInputSurfaceSettings;

/**
Summary:
Get default settings for the Surface structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_VideoImageInput_GetDefaultSurfaceSettings(
    NEXUS_VideoImageInputSurfaceSettings *pSettings /* [out] */
    );



/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_VideoImageInput_GetDefaultSettings(
    NEXUS_VideoImageInputSettings *pSettings /* [out] */
    );

/**
Summary:
Creates a new VideoImageInput interface
**/
NEXUS_VideoImageInputHandle NEXUS_VideoImageInput_Open( /* attr{destructor=NEXUS_VideoImageInput_Close}  */
    unsigned index, /* Specifies instance of NEXUS_VideoImageInputType indicated in pSettings.
                       For example, NEXUS_VideoImageInput_eVfd and index of 2 means VFD2.
                       If particular SOC doesn't support type and instance, an error will be returned. */
    const NEXUS_VideoImageInputSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Close the interface
**/
void NEXUS_VideoImageInput_Close(
    NEXUS_VideoImageInputHandle videoImage
    );

/**
Summary:
Push (submit) a new surface to be added to the image input display queue to be displayed by VDC.

Description:
Call this function to add the passed in surface to the image input display queue. Passed in surface
settings (pSettings) will dictate how many times the given surface is displayed ( displayVsyncs ), whether it
will be put at the head of the queue ( infront ) and all preceding surfaces flushed, and what pts
value is associated with this surface (pts).

**/
NEXUS_Error NEXUS_VideoImageInput_PushSurface(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image, /* attr{null_allowed=y} Pass NULL to flush the surface queue or send EOS
                                  or push stripedSurface depending on NEXUS_VideoImageInputSurfaceSettings.endOfStream
                                  and NEXUS_VideoImageInputSurfaceSettings.stripedSurface */
    const NEXUS_VideoImageInputSurfaceSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Recycle any surfaces that have been dequeued ( and no longer displayed ) from image input queue.

Description:
Call this function to get back surfaces that have been dequeued ( and no longer displayed ) from
the image input queue.

To return more than one surface, pass in a pointer to an array of surfaces, with number of array
elements specified in num_entries.

Number of surfaces freeable will be indicated by the value of num_returned.

**/
NEXUS_Error NEXUS_VideoImageInput_RecycleSurface(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle *recycled,          /* attr{nelem=num_entries;nelem_out=num_returned} */
    size_t num_entries,
    size_t *num_returned
    );

/**
Summary:
Check that 'image' is unused by nexus and can therefore be reused or freed by the application.

Description:
Before checking an image, nexus cleans up any freed surfaces ( a surface which has transitioned from being displayed to not being displayed ).

Returns NEXUS_SUCCESS if 'image' is now unused by nexus
Returns NEXUS_IMAGEINPUT_BUSY if 'image' is still used by nexus
Returns another non-zero return if image clean up had a problem.
**/
NEXUS_Error NEXUS_VideoImageInput_CheckSurfaceCompletion(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image
    );

/**
Summary:
Set a new surface without blocking.

Description:
Setting the next surface does not mean that a previously set surface is now unused by nexus.
Nexus keeps a pipeline of surfaces. You must call NEXUS_VideoImageInput_CheckSurfaceCompletion to verify that a surface
is unused.  This function can only be used when
NEXUS_VideoImageInputSettings.lowDelayMode = true.

To avoid tearing, be aware of the following types of synchronization when setting a new surface:
1) Before calling NEXUS_VideoImageInput_SetNextSurface, ensure all blits into the new surface are complete using NEXUS_Graphics2D_Checkpoint.
2) Before calling NEXUS_VideoImageInput_SetNextSurface, call NEXUS_Surface_Flush(image) if you did any CPU writes to the new surface.
3) Before blitting into the surface and calling NEXUS_VideoImageInput_SetSurface, call NEXUS_VideoImageInput_CheckCompletion to
   check the new surface is available for re-use ( if the surface was previously used ).

Return code must be checked to see if space was available to accept the new surface and the new surface was added to the display queue.

Returns NEXUS_SUCCESS if 'image' is queued ready for display by nexus
Returns NEXUS_IMAGEINPUT_QUEUE_FULL if queue is full and image couldn't be added.
**/
NEXUS_Error NEXUS_VideoImageInput_SetNextSurface(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image /* attr{null_allowed=y} new surface for video input, NULL is used to clear surface and flush the queue */
    );

/**
Summary:
Deprecated: Set new surface and block until any previously displayed surface is unused (freed)

Description:
Use NEXUS_VideoImageInput_SetNextSurface and NEXUS_VideoImageInput_CheckSurfaceCompletion instead.
**/
NEXUS_Error NEXUS_VideoImageInput_SetSurface(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image /* attr{null_allowed=y} new surface for video input, NULL is used to clear surface and flush the queue */
    );

/**
Summary:
Get currenct settings of the ImageInput.
**/
void NEXUS_VideoImageInput_GetSettings(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_VideoImageInputSettings *pSettings /* [out] */
    );

/**
Summary:
Set new settings for the ImageInput.
**/
NEXUS_Error NEXUS_VideoImageInput_SetSettings(
    NEXUS_VideoImageInputHandle imageInput,
    const NEXUS_VideoImageInputSettings *pSettings
    );


/**
Summary:
Returns the abstract NEXUS_VideoInput connector for the video input.
This connector is passed to a VideoWindow to display the image video.

Description:
See Also: NEXUS_VideoWindow_AddInput
**/
NEXUS_VideoInputHandle NEXUS_VideoImageInput_GetConnector(
    NEXUS_VideoImageInputHandle imageInput
    );

typedef struct NEXUS_VideoImageInputStatus
{
    unsigned memcIndex; /* MEMC with RTS for this MFD */
    unsigned secondaryMemcIndex; /* MEMC with RTS for this MFD's split chroma buffer */
} NEXUS_VideoImageInputStatus;

/**
Summary:
Status for ImageInput
**/
NEXUS_Error NEXUS_VideoImageInput_GetStatus(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_VideoImageInputStatus *pStatus
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_VIDEO_IMAGE_INPUT_H__ */

