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
 ******************************************************************************/

#ifndef BAVC_H__
#define BAVC_H__

#include "bchp.h"
#include "bmem.h"
#include "bmma.h"
#include "bfmt.h"
#include "bavc_types.h"
#include "bpxl_plane.h"
#include "bavc_hdmi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The purpose of this common utility is to create a location to contained
shared data structures used to pass data between the various audio/video
porting interface modules. This utility does not contain any functions.

The modules currently using this common utility are
   o MVD - MPEG Video Decoder
   o VDC - Video Display Control
   o XPT - Data Transport
   o AUD - Audio
   o XVD - X Video Decoder

See Also:
   BMVD_Open,
   BVDC_Open
****************************************************************************/
/* xpt related common defines */
#include "bavc_xpt.h"

/***************************************************************************
Summary:
    A structure representing a field/frame of data from the decoder module.

Description:
    This structure is typically used to pass data from the decoder module to
    the VDC module. This should be done every field so that the VDC can
    display the appropriate field with the appropriate parameters.

    The structure is for programming the MFD in BVN.  Notice that both the
    XVD and MVD could prepare the structure for VDC.

    This structure might also represent a graphics surface which is intended
    to be used as a video source to MFD.

See Also:
    BVDC_Source_MpegDataReady_isr
****************************************************************************/
typedef struct BAVC_MFD_Picture
{
	/*----------------------------------------------------------------------*/
	/* Used by all (MVD/XVD/Surface) */

	/* Contains the field polarity of the VDC interrupt driving the callback.
	 * The VDC module provides this interrupt which drives the firmware
	 * to produce field data. This enumeration contains the polarity offscreen
	 * that interrupt and represents what type of field the VDC would
	 * like to feed out of the MPEG feeder. */
	BAVC_Polarity            eInterruptPolarity;

	/* Contains the display veritical refresh rate of the VDC interrupt driving
	 * the callback.  * The VDC module provides this refresh rate which drives
	 * the firmware to produce field data. This enumeration contains the refresh
	 * rate that interrupt and represents what rate the VDC would like to feed
	 * out of the MPEG feeder. */
	BFMT_Vert                eInterruptRefreshRate;

	/* Fast non-real-time (FNRT) support flag, to seed/pre-charge VDC internal
	 * state for modules like deinterlacer. Usually it is set for the overlap part
	 * between chunks n and n+1, preparing for the start of chunk n+1.It is also
	 * meta data relay to ViCE through mailbox by VDC */
	bool                      bPreChargePicture;

	/* Fast non-real-time (FNRT) support flage, to indicate last picture inside
	 * a chunk data. It is also meta data relay to ViCE through mailbox by VDC */
	bool                      bEndOfChunk;

	/* unintentional picture repeat due to decoder underflow at non-real time
	 * transcode mode Encoder FW should simply drop this picture with IGNORE
	 * flag set. No encode time stamp to be adjusted. The flag is generated by
	 * DM software. Ignore should be zero for all other cases.
	 * source: display manager */
	bool                     bIgnorePicture;

	/* video decoder underflow in non-real time transcode mode.  The video
	 * decoder STC should not increment when this flag is true. The flag is
	 * generated by DM software. bStallStc should be false for all other cases.
	 * source: display manager */
	bool                     bStallStc;

	/* DM will set the bLast flag to mark that the last picture decoded from
	 * source has been presented by DM. This flag is used in non-realtime
	 * transcode mode to cleanly flush/terminate transcoder at the end of
	 * source file. When it's true, bIgnorePicture should be true too. */
	bool                     bLast;

	/* Determines if the field is to be muted. This is set when all values
	 * except eInterruptPolarity, bIgnorePicture, bStallStc, bLast and
	 * bChannelChange are invalid and undefined. This flag notifies the VDC
	 * that the feeder should be programmed to feed a fixed constant color
	 * instead of using the supplied buffers. */
	bool                     bMute;

	/* Source polarity.  This indicate that the buffer need to be scan
	 * out as eSourcePolarity (T/B/F polairy).  As determined by the
	 * correct display logic of MVD.  Which could override the original
	 * stream polarity. */
	BAVC_Polarity            eSourcePolarity;

	/* stream panscan vector specifies the position of the centre of
	 * the reconstructed frame from the centre of the display rectangle. */
	/* Units of 1/16th of a pixel in two's complement. Bits 31-16 are
	 * a sign extension of Bit 15. Bit 15 is the sign bit. Bits 14:8
	 * are the macro block grid.Bits 7:4 are the pixel grid. Bits 3:0
	 * are 1/16 pixel grid. */
	int32_t                  i32_HorizontalPanScan;

	/* Units of 1/16th of a pixel in two's complement. Bits 31-16 are
	 * a sign extension of Bit 15. Bit 15 is the sign bit. Bits 14:8
	 * are the macro block grid.Bits 7:4 are the pixel grid. Bits 3:0
	 * are 1/16 pixel grid. */
	int32_t                  i32_VerticalPanScan;

	/* When pan-scan is enabled, this is the horizontal size of the
	 * displayed portion of the buffer. */
	uint32_t                 ulDisplayHorizontalSize;

	/* When pan-scan is enabled, this is the vertical size of the
	 * displayed portion of the buffer. */
	uint32_t                 ulDisplayVerticalSize;

	/* Matrix coefficients. */
	BAVC_MatrixCoefficients  eMatrixCoefficients;

	/* Aspect ratio of the rectangle defined by the display
	 * parameters (ulDisplayHorizontalSize and ulDisplayVerticalSize)
	 * if it's not BFMT_AspectRatio_eSAR; otherwise, look at
	 * uiSampleAspectRatioX/uiSampleAspectRatioY to derive the sample
	 * aspect ratio; */
	BFMT_AspectRatio         eAspectRatio;

	/* Frame rate code. */
	BAVC_FrameRateCode       eFrameRateCode;

	/* When it is a valid graphics surface, the surface is fed as the video source to the video
	 * window. For MPEG Feeder cores with a minor version earlier than 0x60
	 * (see MFD_0_REVISION_ID) and Video Feeder cores with a minor version
	 * earlier than 0x50, the graphics surface for the video
	 * source must have a pixel format of either BPXL_eY18_Cr8_Y08_Cb8,
	 * BPXL_eCr8_Y18_Cb8_Y08, BPXL_eY18_Cb8_Y08_Cr8, or BPXL_eCb8_Y18_Cr8_Y08;
	 * otherwise pink and green video will result. Cores with minor version
	 * 0x60 and 0x50 (for MFD and VFD respectively) and later can support all
	 * formats. Refer to the description of BPXL_Format for details
	 * on the byte location with each pixel format. */
	BPXL_Plane              *pSurface;

	/* Pixel format of given decoded surface.
	 *   ePxlFmt == BPXL_eCr8_Y18_Cb8_Y08
	 *   ePxlFmt == BPXL_eY18_Cr8_Y08_Cb8
	 *   ePxlFmt == BPXL_eY08_Cb8_Y18_Cr8
	 *   ePxlFmt == BPXL_eCb8_Y08_Cr8_Y18

	 *   ePxlFmt == BPXL_eCb8_Y18_Cr8_Y08
	 *   ePxlFmt == BPXL_eY18_Cb8_Y08_Cr8
	 *   ePxlFmt == BPXL_eY08_Cr8_Y18_Cb8
	 *   ePxlFmt == BPXL_eCr8_Y08_Cb8_Y18

	 *  ePxlFmt == BPXL_INVALID be treated as default XVD 420 decoder format. */
	BPXL_Format              ePxlFmt;

	/*----------------------------------------------------------------------*/
	/* MVD and XVD only */

	/* This field gives the Progressive Sequence bit in the stream for MPEG
	 * streams that contain the progressive_sequence bit in the sequence
	 * header. For other protocols, the Display Manager (XVD) may choose to
	 * derive this bit from other parameters. Hence, the field is not
	 * dependable, e.g., in AVC, there is no concept of "prog_seq". It also
	 * can happen that streams marked interlaced are full of Progressive frames.
	 * This is informational only. VDC does not use this flag. */
	bool                     bStreamProgressive;

	/* This field gives the Progressive Frame bit in the stream.
	 * Progressive Frame bit arrives in the Picture header of MPEG
	 * stream and hence may change every frame. For other protocols,
	 * like AVC, it is derived from other coding tools like ct_type
	 * in picture_timing SEI and frame_mbs_only_flag.  This is informational
	 * only VDC does not use this flag. */
	bool                     bFrameProgressive;

	/* Chroma location type. Still use the variable name eMpegType for
	 * backward compatibility. This information is used by the MFD (Feeder)
	 * to decide where chroma position is related to luma position. Protocols
	 * supported include MPEG1, MPEG2, and AVC. */
	BAVC_ChromaLocation      eMpegType;

	/* YCbCr type: 4:2:0, 4:2:2 or 4:4:4. */
	BAVC_YCbCrType           eYCbCrType;

	/* Chrominance interpolation mode. */
	BAVC_InterpolationMode   eChrominanceInterpolationMode;

	/* color primaries */
	BAVC_ColorPrimaries      eColorPrimaries;

    /* preferred transfer characteristics when TV does support */
    BAVC_TransferCharacteristics ePreferredTransferCharacteristics;

	/* transfer characteristics */
	BAVC_TransferCharacteristics eTransferCharacteristics;

	/* The actual horizontal size of the provided buffers (post-clipped). */
	uint32_t                 ulSourceHorizontalSize;

	/* The actual vertical size of the provided buffers (post-clipped). */
	uint32_t                 ulSourceVerticalSize;

	/* If eFrame(=0), it's frame buffer; else if eFieldsPair(=1), it's fields pair. */
	BAVC_DecodedPictureBuffer    eBufferFormat;

	/* Luminance frame buffer address (original buffer address).
	* or luminance top field buffer address when eBufferFormat is eFieldsPair.
	* Or surface address of surface when (ePxlFmt == 422 formats)*/
	BMMA_Block_Handle        hLuminanceFrameBufferBlock;
	uint32_t                 ulLuminanceFrameBufferBlockOffset;

	/* Chrominance frame buffer address (original buffer address).
	* or chrominance top field buffer address when eBufferFormat is eFieldsPair. */
	BMMA_Block_Handle        hChrominanceFrameBufferBlock;
	uint32_t                 ulChrominanceFrameBufferBlockOffset;

	/* Luminance bottom field buffer (new address) if eBufferFormat is eFieldsPair; else, =NULL. */
	BMMA_Block_Handle        hLuminanceBotFieldBufferBlock;
	uint32_t                 ulLuminanceBotFieldBufferBlockOffset;

	/* Chrominance bottom field buffer (new address) when eBufferFormat is eFieldsPair; else, =NULL. */
	BMMA_Block_Handle        hChrominanceBotFieldBufferBlock;
	uint32_t                 ulChrominanceBotFieldBufferBlockOffset;

	/* Adjusted Quantization Parameter to be used for Digital Noise Reduction;
	 * default should be zero; */
	uint32_t                 ulAdjQp;

	/* If scanning out a top field, repeat is set if the previous top field is the same
	 * If scanning out a bottom field, repeat is set if the previous bottom field is the same
	 * If scanning out a progressive frame, repeat is set if the previous progressive frame is the same */
	bool                     bRepeatField;

	/* slow->fast(e.g. 50->60) or trick mode repeat;
	 * it should be set for any true repeat (the source polarity and frame
	 * buffer are the same as the previous picture). It helps VDC/MAD/Multi-buffer
	 * to display better. */
	bool                     bPictureRepeatFlag;

	/* When decoder is in trick mode (fast/pause/slow motion) and its scanout
	 * cadence is interlaced, notify VDC to ignore cadence match. The result
	 * will be  half of fields would be field-inverted by SCL and no more
	 * regular multi-buffer skip/repeat bouncing. When VDC sees the flag be set,
	 * its capture and playback cadence should ignore source cadence match so
	 * the sync-slipped multi-buffer reader/writer pointers would proceed normally
	 * without extra field repeat/skip; otherwise, VDC cadence match is done by
	 * best effort. */
	bool                     bIgnoreCadenceMatch;

	/*---------------------------------------------------------------------
	 * Mosaic Mode:
	 * ------------
	 * Mosaic mode is an option where an existing single decode/display path
	 * is being re-used to support multiple smaller decode/display operations
	 * at full frame-rate.
	 * For each active channel there will be one and only one entry added to
	 * a linked list. The linked list has the following properties:
	 *
	 * Each element in the linked list is a BAVC_XVD_Picture structure. The
	 * element 'pNext' was added to this existing structure to make it into
	 * a linked list. This is the same structure used for the single decode
	 * case so the single decode comes across as a 1-entry linked list.
	 *
	 * If a channel is not open, there will be no entry in the list and the
	 * VDC callback will not be made. If a channel is open (even if
	 * StartDecode? has not been called), there will always be an element on
	 * this list corresponding to this channel and the VDC callback will be
	 * made. Even if there are no buffers to decode, the list element will
	 * exist but bMute will be set to true. There will be no padding of
	 * additional linked list elements to make up for inactive channels. In
	 * other words, if you only have channels 3, 5, and 7 open, the XVD will
	 * not pass muted structures for channel 1, 2, 4, 6, or 8. Instead the
	 * linked list will only have elements for 3, 5, and 7.
	 *
	 * Elements in the list will be sorted in ascending order according to
	 * channel number. There will only be one element for each channel number.
	 */

	/* linked list of mosaic pictures; the last element of the list should
	 * set it to NULL to terminate; */
	void                    *pNext;

	/* channel index for mosaic mode linked list; the index is per decoder
	 * instance based, in other words, decoder 0 has active channels 1, 2, 3,
	 * while decoder 1 could also have different channels 1, 2, 3, which are
	 * independent of decoder 0's channels; */
	uint32_t                 ulChannelId;

	/*----------------------------------------------------------------------*/
	/* MVD or (ePxlFmt == 422 foramts) */

	/* The stride of the luminance and chrominance buffers in bytes measured
	 * from the start of a line's macroblock to the start of the next line's
	 * macroblock.
	 *
	 * Or surface stride of surface when (ePxlFmt == 422 foramts)*/
	uint32_t                 ulRowStride;

	/*----------------------------------------------------------------------*/
	/* XVD only */
	/* This field specifies that the feeder needs to capture the CRC of this
	 * frame, and send it back to the application if callback were registered.
	 * This is for feeder with the capability of doing CRC when feeding.  In
	 * addition if this flag is set by the deocode it also indication that
	 * correct display is not honor, and we may get incorrect display on the
	 * backend. */
	bool                     bCaptureCrc;

	/* Number of macroblocks size in the vertical direction for Luminance
	 * array of the picture to be displayed. */
	uint32_t                 ulLuminanceNMBY;

	/* Number of macroblocks size in the vertical direction for Chrominance
	 * array of the picture to be displayed. */
	uint32_t                 ulChrominanceNMBY;

	/* Stripe width, can be 64 byte or 128 byte */
	BAVC_StripeWidth         eStripeWidth;

	/* Valid when  eAspectRatio == eSAR. */
	uint16_t                 uiSampleAspectRatioX;
	uint16_t                 uiSampleAspectRatioY;

	/* PR 17811 idr_pic_id and PicOrderCnt values passed to VDC during CRC mode.
	 * It is set to Zero otherwise.  The range of int32_PicOrderCnt from
	 * decoder can in the full range of int32_t.  While ulIdrPicID is an
	 * uint32_t, but only 16 bits of unsinged value is used. */
	uint32_t                 ulIdrPicID;
	int32_t                  int32_PicOrderCnt;

	/* PR20042: Need the top/left coordinate of the clipping values from decoder
	 * to calculate new MSTART for MFD.  Unit is in pixel. */
	uint32_t                 ulSourceClipTop;
	uint32_t                 ulSourceClipLeft;

	/* VC1 range re-mapping setting:
	   These values are used for VC1 decode display; */
	uint32_t                 ulLumaRangeRemapping;
	uint32_t                 ulChromaRangeRemapping;

	/* This field indicate if valid AFD present present in the stream or not.
	 * if bValidAfd=true, then ulAfd can then be used. */
	bool                     bValidAfd;

	/* AFD values defined by User data registered by ITU-T Recommendation T.35
	 * SEI message (see ISO/IEC 14496-10:2005, Subclauses D.8.5 and D.9.5).
	 * The AFD value describe the area of interest of the stream. */
	uint32_t                 ulAfd;

	/* specify top/bottom or left/right data */
	BAVC_BarDataType         eBarDataType;

	/* either the top or left bar data value, as defined above */
	uint32_t                 ulTopLeftBarValue;

	/* either the bottom or right bar data value, as defined above */
	uint32_t                 ulBotRightBarValue;

	/* 3D orientation */
	BFMT_Orientation         eOrientation;

	/* 8 or 10-bit depth */
	BAVC_VideoBitDepth       eBitDepth;

	/* Original PTS of the picture, relayed by STG to ViCE2 Mailbox.
	 * Note, this PTS value could be either original coded PTS or interolated
	 * PTS by DM. */
	uint32_t                 ulOrigPTS;

	/* Original input Picture type, relayed by STG to ViCE2 Mailbox.
	 * Decoder FW extracts the value and passes to DM via PPB structure.
	 * source: bxvd_vdec_info.h  structure BXVD_P_PPB */
	BAVC_PictureCoding       ePictureType;

	/* Should be set to "true" for all pictures sent to VDC before the first
	 * decoded picture is available after a channel is stopped (for both muted
	 * or repeated hold last pic frames).  See also SW7425-2239and SW7425-2253. */
	bool                     bChannelChange;

	/* Decoded picture id for userdata transcode */
	uint32_t                 ulDecodePictureId;

	/* Fast non-real-time (FNRT) support. 10-bit chunk index to indicate which
	 * chunk the picture belongs to. It is also meta data relay to ViCE by VDC */
	uint32_t                 ulChunkId;

	/* Pointer to BAVC_MFD_Picture specify pictures that must be processed at
	 * the same time to form a complete display */
	void                    *pEnhanced;

	/* Content Light SEI message */
	uint32_t                ulAvgContentLight; /* Pic Average Light level */
	uint32_t                ulMaxContentLight; /* Max Light level */

	/* Mastering Display Colour Volume */
	BAVC_Point              stDisplayPrimaries[3];
	BAVC_Point              stWhitePoint;
	uint32_t                ulMaxDispMasteringLuma;
	uint32_t                ulMinDispMasteringLuma;

} BAVC_MFD_Picture;

typedef BAVC_MFD_Picture BAVC_XVD_Picture;   /* AVC decoder */
typedef BAVC_MFD_Picture BAVC_MVD_Picture;   /* MiniTitan decoder */

/* Backward compatibility, for legacy code. */
typedef BAVC_MFD_Picture BAVC_MVD_Field;      /* MiniTitan decoder */


/***************************************************************************
Summary:
    Contains the picture info for the HDDVI input port.

Description:
    This structure is used to pass data from the XVD module to the
    VDC module, through a call-back. Fields that are unused, simply
    means XVD doesn't have the data, and VDC doesn't expect any.
    Naming convention used: BAVC_To_Source_Name

See Also:
    BVDC_Source_PictureCallback_isr, BVDC_Source_InstallPictureCallback
****************************************************************************/
typedef struct BAVC_VDC_HdDvi_Picture
{
	/* Determines if the field is to be muted. This is set when all other
	   values in this structure are invalid and undefined. This flag
	   notifies the VDC that the feeder should be programmed to feed a
	   fixed constant color instead of using the supplied buffers. */
	bool                     bMute;

	/* This field gives the Progressive Sequence bit in the stream.
	   Progressive Sequence bit comes in the Sequence header of a
	   stream and does not change every frame. It also can happen
	   that streams marked interlaced are full of Progressive frames
	   This is _only_ for information.
	*/
	bool                     bStreamProgressive;

	/* XVD tells VDC to detect sync pulses */
	bool                     bResetHdDviBegin;
	bool                     bResetHdDviEnd;

	/* XVD tells VDC to change new hddvi format, if bNeedSync is true */
	/* TODO: Fix this to BFMT_VideoFmt later */
	uint32_t                 eHddviFormat;

	/* Difference between STC and PTS of a given frame at Vsync time */
	long                     lPtsStcOffset;

	/* Source polarity!  To be field by VDC!  Taken from slot interrupt.
	 * HDR or XVD does not need to initialized this field. */
	BAVC_Polarity            eSourcePolarity;

	/* Units of 1/16th of a pixel in two's complement. Bit 15 is the
	 * sign bit. Bits 14:8 are the macro block grid.Bits 7:4 are the
	 * pixel grid. Bits 3:0 are 1/16 pixel grid. */
	int32_t                  i32_HorizontalPanScan;

	/* Units of 1/16th of a pixel.in two's complement. Bit 15 is the
	 * sign bit. Bits 14:8 are the macro block grid. Bits 7:4 are the
	 * pixel grid. Bits 3:0 are 1/16 pixel grid. */
	int32_t                  i32_VerticalPanScan;

	/* Color space type: RGB, YCbCr444 and YCbCr422 */
	BAVC_Colorspace          eColorSpace;

	/* Color depth: 8, 10 or 12 bit */
	BAVC_HDMI_BitsPerPixel   eColorDepth;

	/* RGB Quantization */
	BAVC_CscMode             eCscMode;

	/* YCbCr type. */
	BAVC_YCbCrType           eYCbCrType;

	/* color primaries */
	BAVC_ColorPrimaries      eColorPrimaries;

	/* transfer characteristics */
	BAVC_TransferCharacteristics eTransferCharacteristics;

	/* Matrix coefficients. */
	BAVC_MatrixCoefficients  eMatrixCoefficients;

	/* Aspect ratio of the rectangle defined by the display
	 * parameters (ulDisplayHorizontalSize and ulDisplayVerticalSize) */
	BFMT_AspectRatio         eAspectRatio;

	/* Sample aspect ratio for AVC */
	uint16_t                 uiSampleAspectRatioX;
	uint16_t                 uiSampleAspectRatioY;

	/* Frame rate code. */
	BAVC_FrameRateCode       eFrameRateCode;

	/* When pan-scan is enabled, this is the horizontal size of the
	 * displayed portion of the buffer. */
	uint32_t                 ulDisplayHorizontalSize;

	/* When pan-scan is enabled, this is the vertical size of the
	 * displayed portion of the buffer. */
	uint32_t                 ulDisplayVerticalSize;

	/* The actual horizontal size of the provided buffers. */
	uint32_t                 ulSourceHorizontalSize;

	/* The actual vertical size of the provided buffers. */
	uint32_t                 ulSourceVerticalSize;

	/* For HDMI Mode, the contents of AVI Infoframe should be used to set
	 * the value of the pixel decimation circuit rather than have it calculated */
	bool                     bHdmiMode;
	uint32_t                 ulPixelRepitionFactor;

} BAVC_VDC_HdDvi_Picture;

/***************************************************************************
Summary:
    A structure representing a graphics frame.

Description:
    This structure is used to pass a graphics frame to BVDC for display, from
    BGRC or BP3D.

    If the optional alpha surface is not needed, hAlphaSurface should be set
    to NULL.

See Also:
    BVDC_Source_PictureCallback_isr, BVDC_Source_InstallPictureCallback
    BVDC_Source_SetSurface, BVDC_Source_SetAlphaSurface
****************************************************************************/
typedef struct BAVC_Gfx_Picture
{
	const BPXL_Plane *pSurface;       /* main surface */
	const BPXL_Plane *pRSurface;      /* main R surface */

	/* 3D orientation */
	BFMT_Orientation     eInOrientation;

	/* Original PTS of the picture, relayed by STG to ViCE2 Mailbox. */
	uint32_t             ulOrigPTS;

	/* striped picture */
	BAVC_MFD_Picture	*pstMfdPic;
} BAVC_Gfx_Picture;

/***************************************************************************
Summary:
    Return the default BAVC_MatrixCoefficients

Description:
    This function returns the default BAVC_MatrixCoefficients (i.e. color space
    standard) according to display format and bXvYcc.

See Also:
    BFMT_VideoFmt, BAVC_MatrixCoefficients
****************************************************************************/
BAVC_MatrixCoefficients  BAVC_GetDefaultMatrixCoefficients_isrsafe(
	BFMT_VideoFmt eDisplayFmt,
	bool          bXvYcc );

/***************************************************************************
Summary:
    Return eotf

Description:
    This function returns the BAVC_HDMI_DRM_EOTF according to
    BAVC_TransferCharacteristics.

See Also:
    BAVC_TransferCharacteristics
****************************************************************************/
BAVC_HDMI_DRM_EOTF BAVC_TransferCharacteristicsToEotf_isrsafe(
	BAVC_TransferCharacteristics eTransChar);


/* vce related common defines */
#include "bavc_vce.h"

/* rap related common defines */
#include "bavc_rap.h"


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BAVC_H__ */

/* end of file */
