/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * File Description: This file definitions for PPB data structure passed
 *                   from 7401 decode FW to DM.
 *
 ***************************************************************************/

#ifndef __INC_VDEC_INFO_H__
#define __INC_VDEC_INFO_H__

/* version information for PPB and other ancilliary data */
#define VDEC_INFO_VERSION               1.8

/* stripe-width used for output of decoded pixels */
#define VIDEO_STRIPE_WIDTH              64

/* buffer alignment required by hardware for luma, chroma in bytes */
#define FRAME_BUFFER_ALIGNMENT          (4 * 1024)


#if BXVD_P_FW_HIM_API

#define BXVD_P_PPB_EXTENDED 1

#endif

/*
 * SW7405-3996:: Constant and structure definitions for SEI messages.
 */

/*
 *  Types of SEI messages
 */
#define  BXVD_P_PPB_SEI_MSG_FRAMEPACKING           0x2D
#define  BXVD_P_PPB_SEI_MSG_MVC_GRAPHICS_OFFSET    0x80


/* Graphics offset message */

#define BXVD_P_MAX_MVC_OFFSET_META_SEQ 32

typedef struct
{
    int32_t         size;
    unsigned char   offset[BXVD_P_MAX_MVC_OFFSET_META_SEQ];
} BXVD_P_MVC_Offset_Meta;


/* Frame packing message */

typedef struct
{
   uint32_t flags;
   uint32_t frame_packing_arrangement_id;
   uint32_t frame_packing_arrangement_type;
   uint32_t content_interpretation_type;
   uint32_t frame0_grid_position_x;
   uint32_t frame0_grid_position_y;
   uint32_t frame1_grid_position_x;
   uint32_t frame1_grid_position_y;
   uint32_t frame_packing_arrangement_reserved_byte;
   uint32_t frame_packing_arrangement_repetition_period;

} BXVD_P_SEI_FramePacking;

/* Constants for interpreting the "flags" field in BXVD_P_SEI_FramePacking
 *
 * "arrangement_cancel":
 * The H264 specification was not completely rigorous in the definition of this bit.  We are assuming
 * that if this bit is set, none of the other fields in the message are valid.   This then implies
 * that the picture is a 2D picture.  The orientation will be set to "2D" in both the Unified and
 * MFD picture structures. (SW7405-5135)
 */
#define BXVD_P_PPB_FLAG_SEI_FRAMEPACK_ARRANGEMENT_CANCEL          (0x00000001)      /* if set, implies 2D */
#define BXVD_P_PPB_FLAG_SEI_FRAMEPACK_QUINCUNX_SAMPLING           (0x00000002)
#define BXVD_P_PPB_FLAG_SEI_FRAMEPACK_SPATIAL_FLIPPING            (0x00000004)
#define BXVD_P_PPB_FLAG_SEI_FRAMEPACK_FRAME0_FLIPPED              (0x00000008)
#define BXVD_P_PPB_FLAG_SEI_FRAMEPACK_FIELD_VIEWS                 (0x00000010)
#define BXVD_P_PPB_FLAG_SEI_FRAMEPACK_CURRENT_FRAME_IS_FRAME0     (0x00000020)
#define BXVD_P_PPB_FLAG_SEI_FRAMEPACK_FRAME0_SELF_CONTAINED       (0x00000040)
#define BXVD_P_PPB_FLAG_SEI_FRAMEPACK_FRAME1_SELF_CONTAINED       (0x00000080)
#define BXVD_P_PPB_FLAG_SEI_FRAMEPACK_ARRANGEMENT_EXTENSION       (0x00000100)


/* Constants for interpreting the "frame_packing_arrangement_type" field in BXVD_P_SEI_FramePacking
 */
#define BXVD_P_PPB_SEI_FRAMEPACK_TYPE_CHECKER        0
#define BXVD_P_PPB_SEI_FRAMEPACK_TYPE_COLUMN         1
#define BXVD_P_PPB_SEI_FRAMEPACK_TYPE_ROW            2
#define BXVD_P_PPB_SEI_FRAMEPACK_TYPE_SIDE_BY_SIDE   3
#define BXVD_P_PPB_SEI_FRAMEPACK_TYPE_OVER_UNDER     4
#define BXVD_P_PPB_SEI_FRAMEPACK_TYPE_ALTERNATING    5

/* Constants for interpreting the "content_interpretation_type" field in BXVD_P_SEI_FramePacking
 */
#define BXVD_P_PPB_SEI_FRAMEPACK_INTERPRET_UNSPECIFIED  0
#define BXVD_P_PPB_SEI_FRAMEPACK_INTERPRET_FRAME0_LEFT  1
#define BXVD_P_PPB_SEI_FRAMEPACK_INTERPRET_FRAME0_RIGHT 2

/* Generic wrapper for messages delivered by AVD */

typedef struct BXVD_P_SEI_Message
{
   uint32_t uiMsgType;

   /* Pointer to next message if sent. */

/*   struct BXVD_P_SEI_Message * pstNextSEIMsgOffset;*/
   uint32_t pstNextSEIMsgOffset;

   union
   {
      BXVD_P_MVC_Offset_Meta  stOffsetMeta;
      BXVD_P_SEI_FramePacking stSEIFramePacking;

   } data;

}  BXVD_P_SEI_Message;

/*
 * end:: SW7405-3996 add SEI messages
 */

/* SW7425-2247: constants for interpreting the "bar_data" field
 *
 * bits 13:0   top or left Bar Data
 * bits 15:14  unused
 * bits 29:16  bottom or right Bar Data
 * bit 30      1 means Bar data is Top_Bottom, 0 means Bar Data is Left_Right
 * bit 31      1 means Bar data is valid, 0 means invalid
 */
#define BXVD_P_PPB_BAR_DATA_TOP_LEFT_MASK    (0x00003FFF)
#define BXVD_P_PPB_BAR_DATA_TOP_LEFT_SHIFT   0

#define BXVD_P_PPB_BAR_DATA_BOT_RIGHT_MASK   (0x3FFF0000)
#define BXVD_P_PPB_BAR_DATA_BOT_RIGHT_SHIFT  16

#define BXVD_P_PPB_BAR_DATA_FLAG_TOP_BOTTOM  (0x40000000)
#define BXVD_P_PPB_BAR_DATA_FLAG_VALID       (0x80000000)


/* User Data Header */
typedef struct user_data
{
   uint32_t      next; /* FW has struct user_data  *next; which is a 32 bit value */
   uint32_t      type;
   uint32_t      size;
} UD_HDR;

/*------------------------------------------------------*
 *    MPEG Extension to the BXVD_P_PPB                  *
 *------------------------------------------------------*/
#define  BXVD_P_PPB_MPEG_USERDATA_TYPE_SEQ           (1)
#define  BXVD_P_PPB_MPEG_USERDATA_TYPE_GOP           (2)
#define  BXVD_P_PPB_MPEG_USERDATA_TYPE_PIC           (4)
#define  BXVD_P_PPB_MPEG_USERDATA_TYPE_TOP           (8)
#define  BXVD_P_PPB_MPEG_USERDATA_TYPE_BOT           (16)
#define  BXVD_P_PPB_MPEG_USERDATA_TYPE_I             (32)
#define  BXVD_P_PPB_MPEG_USERDATA_TYPE_P             (64)
#define  BXVD_P_PPB_MPEG_USERDATA_TYPE_B             (128)

/* GOP time code field extraction */
#define BXVD_P_PPB_MPEG_GOP_HOUR_MASK    0xf80000
#define BXVD_P_PPB_MPEG_GOP_MINUTE_MASK  0x7e000
#define BXVD_P_PPB_MPEG_GOP_SECOND_MASK  0xfc0
#define BXVD_P_PPB_MPEG_GOP_PICTURE_MASK 0x3f
#define BXVD_P_PPB_MPEG_GOP_HOUR_SHIFT   0x13
#define BXVD_P_PPB_MPEG_GOP_MINUTE_SHIFT 0x0d
#define BXVD_P_PPB_MPEG_GOP_SECOND_SHIFT 0x06

typedef struct
{
   uint32_t           pictype;   /* pict_coding_type (I=1, P=2, B=3) */

   /* Always valid,  defaults to picture size if no
      sequence display extension in the stream. */
   uint32_t      display_horizontal_size;
   uint32_t      display_vertical_size;

   /* MPEG_VALID_PANSCAN
      Offsets are a copy values from the MPEG stream. */
   uint32_t      offset_count;
   int32_t       horizontal_offset[3];
   int32_t       vertical_offset[3];

   /* GOP time code [23:19]=hour, [18:13]=minute, [11:6]=sec, [5:0]=frame */
   uint32_t gop_time_code;

   uint32_t      bit_rate_value;         /* bit-rate from sequence header */
   uint32_t      low_delay_video_format; /* [17:16]=low_delay, [2:0]=video_format */
   uint32_t      frame_rate_extension;   /* [6:5]=frame_rate_extn_n, [4:0]=frame_rate_extn_d */
   unsigned char     repeat_first_field;    /*  MPEG-2 repeat first flag */
   unsigned char     reserved[3];

} BXVD_P_PPB_MPEG;


/*------------------------------------------------------*
 *    VC1 Extension to the PPB                          *
 *------------------------------------------------------*/
#define  BXVD_P_PPB_VC1_VALID_PANSCAN             (1)
#define  BXVD_P_PPB_VC1_VALID_USER_DATA           (2)
#define  BXVD_P_PPB_VC1_USER_DATA_OVERFLOW        (4)

#define  BXVD_P_PPB_VC1_USERDATA_TYPE_SEQ        (1)
#define  BXVD_P_PPB_VC1_USERDATA_TYPE_ENTRYPOINT (2)
#define  BXVD_P_PPB_VC1_USERDATA_TYPE_FRM        (4)
#define  BXVD_P_PPB_VC1_USERDATA_TYPE_FLD        (8)
#define  BXVD_P_PPB_VC1_USERDATA_TYPE_SLICE      (16)

typedef struct
{
   /* Always valid,  defaults to picture size if no
      sequence display extension in the stream. */
   uint32_t      display_horizontal_size;
   uint32_t      display_vertical_size;

  /* VC1 pan scan windows
   */
   uint32_t      num_panscan_windows;
   uint32_t      ps_horiz_offset_width[4]; /* bits[31:14]=horiz_offset, [13:0]=width */
   uint32_t      ps_vert_offset_height[4]; /* bits[31:14]=vert_offset, [13:0]=height */

   uint32_t      range_remapping_ratio;    /* expansion/reduction information */

} BXVD_P_PPB_VC1;


/*------------------------------------------------------*
 *    AVS Extension to the BXVD_P_PPB                   *
 *------------------------------------------------------*/
#define  BXVD_P_PPB_AVS_VALID_PANSCAN             (1)
#define  BXVD_P_PPB_AVS_VALID_USER_DATA           (2)

#define  BXVD_P_PPB_AVS_USER_DATA_TYPE_SEQ        (1)
#define  BXVD_P_PPB_AVS_USER_DATA_TYPE_FRM        (2)
#define  BXVD_P_PPB_AVS_USER_DATA_OVERFLOW        (4)

typedef struct
{
   /* Always valid,  defaults to picture size if no
      sequence display extension in the stream. */
   uint32_t      display_horizontal_size;
   uint32_t      display_vertical_size;

   /* AVS_VALID_PANSCAN
      Offsets are a copy values from the AVS stream. */
   uint32_t      offset_count;
   int32_t       horizontal_offset[3];
   int32_t       vertical_offset[3];

} BXVD_P_PPB_AVS;

/* can these MAX_FGT_xxx constants be deleted? 12/13/12 */
/**
 * @brief Film grain SEI message.
 *
 * Content of the film grain SEI message.
 */
/*maximum number of model-values as for Thomson spec(standard says 5)*/
#define MAX_FGT_MODEL_VALUE         (3)
/* maximum number of intervals(as many as 256 intervals?) */
#define MAX_FGT_VALUE_INTERVAL      (256)

/* This is 3+3+(9*256)+(3*256)+(3*256) */
#define MAX_FGT_VALS       3846

/*------------------------------------------------------*/
/*    H.264 Extension to the BXVD_P_PPB                 */
/*------------------------------------------------------*/

/* userdata type flags */
#define BXVD_P_PPB_H264_USERDATA_TYPE_REGISTERED      4
#define BXVD_P_PPB_H264_USERDATA_TYPE_TOP             8
#define BXVD_P_PPB_H264_USERDATA_TYPE_BOT            16
#define BXVD_P_PPB_H264_USERDATA_TYPE_FRAME_PACK     45

/* Bit definitions for 'other.h264.valid' field */
#define  BXVD_P_PPB_H264_VALID_PANSCAN             (1)
#define  BXVD_P_PPB_H264_VALID_SPS_CROP            (2)
#define  BXVD_P_PPB_H264_VALID_VUI                 (4)

/* SW7401-4426: Bit defintions for the "time_code" field
 * [24:20]=hour, [19:14]=minute, [13:8]=sec, [7:0]=frame
 */
#define BXVD_P_PPB_H264_GOP_HOUR_MASK    0x01F00000
#define BXVD_P_PPB_H264_GOP_MINUTE_MASK  0x000FC000
#define BXVD_P_PPB_H264_GOP_SECOND_MASK  0x00003F00
#define BXVD_P_PPB_H264_GOP_PICTURE_MASK 0x000000FF
#define BXVD_P_PPB_H264_GOP_HOUR_SHIFT   20
#define BXVD_P_PPB_H264_GOP_MINUTE_SHIFT 14
#define BXVD_P_PPB_H264_GOP_SECOND_SHIFT 8

typedef struct
{
   /* 'valid' specifies which fields (or sets of
    * fields) below are valid.  If the corresponding
    * bit in 'valid' is NOT set then that field(s)
    * is (are) not initialized. */
   uint32_t      valid;

   int32_t        poc_top;
   int32_t        poc_bottom;
   uint32_t      idr_pic_id;

   /* H264_VALID_PANSCAN */
   uint32_t      pan_scan_count;
   uint32_t      pan_scan_horiz [3]; /* [31:16]=left, [15:0]=right */
   uint32_t      pan_scan_vert  [3]; /* [31:16]=top, [15:0]=bottom */

   /* H264_VALID_SPS_CROP */
   uint32_t      sps_crop_horiz;     /* [31:16]=left, [15:0]=right */
   uint32_t      sps_crop_vert;      /* [31:16]=top, [15:0]=bottom */

   /* H264_VALID_VUI */
   uint32_t      chroma_top;
   uint32_t      chroma_bottom;

   /* Offset to a BXVD_P_SEI_Message for the current frame */
   uint32_t      pstSEIMessageOffset;

} BXVD_P_PPB_H264;

/*------------------------------------------------------*/
/*  H265 Extension to the BXVD_P_PPB                 */
/*------------------------------------------------------*/

/* SW7445-103: for now simply treat H265 PPB's the same as H264 */

/* userdata type flags */
#define BXVD_P_PPB_H265_USERDATA_TYPE_REGISTERED   BXVD_P_PPB_H264_USERDATA_TYPE_REGISTERED
#define BXVD_P_PPB_H265_USERDATA_TYPE_TOP          BXVD_P_PPB_H264_USERDATA_TYPE_TOP
#define BXVD_P_PPB_H265_USERDATA_TYPE_BOT          BXVD_P_PPB_H264_USERDATA_TYPE_BOT
#define BXVD_P_PPB_H265_USERDATA_TYPE_FRAME_PACK   BXVD_P_PPB_H264_USERDATA_TYPE_FRAME_PACK

#if 0
#define  H265_FRAME_PACK_UD 45
#endif

/* Bit definitions for 'other.h265.valid' field */
#define  BXVD_P_PPB_H265_VALID_PANSCAN             (1)
#define  BXVD_P_PPB_H265_VALID_SPS_CROP            (2)
#define  BXVD_P_PPB_H265_VALID_VUI                 (4)
#define  BXVD_P_PPB_H265_VALID_PIC_WIDTH_HEIGHT    (8)

typedef struct
{
   /* 'valid' specifies which fields (or sets of
    * fields) below are valid.  If the corresponding
    * bit in 'valid' is NOT set then that field(s)
    * is (are) not initialized. */
   unsigned int      valid;

   /* H265_VALID_PANSCAN */
   unsigned int      pan_scan_count;
   unsigned int      pan_scan_horiz [3]; /* [31:16]=left, [15:0]=right */
   unsigned int      pan_scan_vert  [3]; /* [31:16]=top, [15:0]=bottom */

   /* H265_VALID_SPS_CROP */
   unsigned int      sps_crop_horiz;     /* [31:16]=left, [15:0]=right */
   unsigned int      sps_crop_vert;      /* [31:16]=top, [15:0]=bottom */

   /* H265_VALID_VUI */
   unsigned int      chroma_top;
   unsigned int      chroma_bottom;

   unsigned int content_light;          /* [31:16] = Max Pic Avg Light level, [15:0] = Max Content Light level */

   /* Mastering Display Colour Volume */
   unsigned int display_primaries_xy[3]; /* For each component [31:16] = Y and [15:0] = X */
   unsigned int white_point_xy;          /* [31:16] = Y and [15:0] = X */
   unsigned int max_disp_mastering_lum;
   unsigned int min_disp_mastering_lum;
   unsigned int hdr_transfer_characteristics_idc;  /* SWSTB-1629: */
} BXVD_P_PPB_H265;

#define HDR_SEI_CONTENT_LIGHT_AVG_MASK   0xffff0000
#define HDR_SEI_CONTENT_LIGHT_AVG_SHIFT  16

#define HDR_SEI_CONTENT_LIGHT_MAX_MASK   0x0000ffff
#define HDR_SEI_CONTENT_LIGHT_MAX_SHIFT  0

#define HDR_SEI_X_MASK   0x0000ffff
#define HDR_SEI_X_SHIFT  0

#define HDR_SEI_Y_MASK   0xffff0000
#define HDR_SEI_Y_SHIFT  16

/*------------------------------------------------------*/
/*   SVC Extension to the BXVD_P_PPB                 */
/*------------------------------------------------------*/

#define  SVC_VALID_PANSCAN             (1)
#define  SVC_VALID_SPS_CROP            (2)
#define  SVC_VALID_VUI                 (4)

typedef struct
{
   /* 'valid' specifies which fields (or sets of
   * fields) below are valid.  If the corresponding
   * bit in 'valid' is NOT set then that field(s)
   * is (are) not initialized. */
   uint32_t     valid;

   int32_t       poc_top;
   int32_t       poc_bottom;
   uint32_t     idr_pic_id;

   /* SVC_VALID_PANSCAN */
   uint32_t     pan_scan_count;
   uint32_t     pan_scan_horiz[3];  /* [31:16]=left, [15:0]=right */
   uint32_t     pan_scan_vert[3];   /* [31:16]=top, [15:0]=bottom */

   /* SVC_VALID_SPS_CROP */
   uint32_t     sps_crop_horiz;     /* [31:16]=left, [15:0]=right */
   uint32_t     sps_crop_vert;      /* [31:16]=top, [15:0]=bottom */

   /* SVC_VALID_VUI */
   uint32_t     chroma_top;
   uint32_t     chroma_bottom;

} BXVD_P_PPB_SVC;


/*------------------------------------------------------*/
/*   MPEG-4 Part-2 Extension to the BXVD_P_PPB          */
/*------------------------------------------------------*/

typedef struct
{
   uint32_t pictype;   /* pict_coding_type (I=0, P=1, B=2, S=3) */

} BXVD_P_PPB_MP4;

/*------------------------------------------------------*/
/*    VP7/VP8 Extension to the BXVD_P_PPB               */
/*------------------------------------------------------*/

typedef struct
{
   /* 0: No upscaling (the most common case)
    * 1: Upscale by 5/4
    * 2: Upscale by 5/3
    * 3: Upscale by 2
    */
   uint32_t    horiz_scale_mode;
   uint32_t    vert_scale_mode;

   /* SWDTV-8681: add support for VP8 display size */
   uint32_t display_horizontal_size;
   uint32_t display_vertical_size;

} BXVD_P_PPB_VP8;

/*------------------------------------------------------*
 *    VP9 Extension to PPB                              *
 *------------------------------------------------------*/
typedef struct
{
   uint32_t   frame_width;     /* intended decoded frame width, use it as crop windows in picture buffer  */
   uint32_t   frame_height;    /* intended decoded frame height, used it as corp windows in pictue buffer  */
   uint32_t   display_width;   /* display width */
   uint32_t   display_height;  /* display height */
} BXVD_P_PPB_VP9;

/*------------------------------------------------------*/
/*    RV9 Extension to the BXVD_P_PPB                   */
/*------------------------------------------------------*/

typedef struct
{
   /* Always valid, defaults to picture size if no
    * sequence display extension in the stream.
    */
    uint32_t   crop_horiz;     /* [31:16]=left, [15:0]=right */
    uint32_t   crop_vert;      /* [31:16]=top, [15:0]=bottom */

} BXVD_P_PPB_RV9;


/*------------------------------------------------------*
 *    Picture Parameter Block                           *
 *------------------------------------------------------*/

/* Bit definitions for 'flags' field */


#define  BXVD_P_PPB_FLAG_BUFFER_TYPE_MASK     (0x00000003)
#define  BXVD_P_PPB_FLAG_FRAME                (0x00000000)
#define  BXVD_P_PPB_FLAG_FIELDPAIR            (0x00000001)
#define  BXVD_P_PPB_FLAG_TOPFIELD             (0x00000002)
#define  BXVD_P_PPB_FLAG_BOTTOMFIELD          (0x00000003)

#define  BXVD_P_PPB_FLAG_SOURCE_TYPE_MASK     (0x0000000c)
#define  BXVD_P_PPB_FLAG_PROGRESSIVE_SRC      (0x00000000)
#define  BXVD_P_PPB_FLAG_INTERLACED_SRC       (0x00000004)
#define  BXVD_P_PPB_FLAG_UNKNOWN_SRC          (0x00000008)

#define  BXVD_P_PPB_FLAG_BOTTOM_FIRST         (0x00000010)

#define  BXVD_P_PPB_FLAG_PTS_PRESENT          (0x00000020)
#define  BXVD_P_PPB_FLAG_PCR_OFFSET_PRESENT   (0x00000040)
#define  BXVD_P_PPB_FLAG_DISCONT_PCR_OFFSET   (0x00000080)
#define  BXVD_P_PPB_FLAG_PICTURE_LESS_PPB     (0x00000100)
#define  BXVD_P_PPB_FLAG_PROG_SEQUENCE        (0x00000200)
#define  BXVD_P_PPB_FLAG_PIC_TAG_PRESENT      (0x00000400)
#define  BXVD_P_PPB_FLAG_DECODE_ERROR         (0x00000800)

#define  BXVD_P_PPB_FLAG_AFD_VALID            (0x00001000)

/* Bit 14:13 is a 2 bit enum for Picture type
 * 00 - Undefined (backward compatibility)
 * 01 - I Picture
 * 10 - P Picture
 * 11 - B Picture
 */
#define BXVD_P_PPB_FLAG_PICTURE_TYPE_MASK     (0x00006000)
#define BXVD_P_PPB_FLAG_I_PICTURE             (0x00002000)
#define BXVD_P_PPB_FLAG_P_PICTURE             (0x00004000)
#define BXVD_P_PPB_FLAG_B_PICTURE             (0x00006000)

#define BXVD_P_PPB_FLAG_REF_PICTURE           (0x00008000)
#define BXVD_P_PPB_FLAG_RAP_PICTURE           (0x00010000)

#define BXVD_P_PPB_NEW_PIC_TAG_AVAIL          (0x00020000)
#define BXVD_P_PPB_OVERSCAN_FLAG              (0x00040000)
#define BXVD_P_PPB_OVERSCAN_APPROPRIATE_FLAG  (0x00080000)
#define BXVD_P_PPB_FLAG_ERROR_CONCEAL         (0x00100000)
#define BXVD_P_PPB_FLAG_DECODE_REF_ERROR      (0x00200000)
#define BXVD_P_PPB_FLAG_INPUT_OVERFLOW        (0x00400000)

/*
 * Bits 26:23: ( a 4-bit value of 0 means 2D PPB)
 *  These bits (enum) are used to indicate which view component PPB belongs to
 * Bit 26: When set to 1, indicates MVC Base View component, when set to 0
 *     indicates dependent view component
 * Bit 25:23: This three bit number indicates the view index number for the
 *     dependent view. This ranges from 1 to 7. 0 is not used, as Base view is
 *     view index 0. This number is only valid when Bit 26 is set to 0.
 *
 */
#define BXVD_P_PPB_MULTIVIEW_FIELD_MASK       (0x07800000)
#define BXVD_P_PPB_MULTIVIEW_COUNT_MASK       (0x03800000)
#define BXVD_P_PPB_MULTIVIEW_COUNT_SHIFT      23
#define BXVD_P_PPB_MULTIVIEW_BASE_FLAG        (0x04000000)

#define BXVD_P_PPB_FLAG_FIXED_FRAME_RATE      (0x08000000)

/* SW7405-4560:: Indicates that this is the first time the message is being delivered.
 * The 3D framepacking messages can be repeated for a 'N' pictures, this
 * flag can be used to filter repeated messages.
 */
#define BXVD_P_PPB_FLAG_NEW_SEI_MSG           (0x10000000)


#if BXVD_P_PPB_EXTENDED

/* Bit definitions for the 'flags_ext0' field */

/* SW7425-1001: ..._LAST_PICTURE effectively an EOS flag.
 * Currently defined to only be delivered with a "picture-less" PPB.
 * SW7445-586: added BXVD_P_PPB_EXT0_FLAG_DUPLICATE_FIELD for
 * HEVC interlaced content. For 3:2 content, indicates that the
 * third field is a repeat and can be dropped.
 */
#define BXVD_P_PPB_EXT0_FLAG_LAST_PICTURE          (0x00000001)
#define BXVD_P_PPB_EXT0_FLAG_LUMA_10_BIT_PICTURE   (0x00000002)
#define BXVD_P_PPB_EXT0_FLAG_CHROMA_10_BIT_PICTURE (0x00000004)
#define BXVD_P_PPB_EXT0_FLAG_DUPLICATE_FIELD       (0x00000008)

#endif


/* Values for 'pulldown' field.  '0' means no pulldown information
 * was present for this picture.
 * SW7445-586: added ..._e(Top/Bottom)(Prev/Next)(Bottom/Top) for
 * HEVC interlaced content.  The PPB's will contain a single field.
 * The flags will be used to pair up the fields.
 */
typedef enum BXVD_P_PPB_PullDown
{
  BXVD_P_PPB_PullDown_eTop             = 1,
  BXVD_P_PPB_PullDown_eBottom          = 2,
  BXVD_P_PPB_PullDown_eTopBottom       = 3,
  BXVD_P_PPB_PullDown_eBottomTop       = 4,
  BXVD_P_PPB_PullDown_eTopBottomTop    = 5,
  BXVD_P_PPB_PullDown_eBottomTopBottom = 6,
  BXVD_P_PPB_PullDown_eFrameX2         = 7,
  BXVD_P_PPB_PullDown_eFrameX3         = 8,
  BXVD_P_PPB_PullDown_eFrameX1         = 9,
  BXVD_P_PPB_PullDown_eFrameX4         = 10,
  BXVD_P_PPB_PullDown_eTopPrevBottom   = 11,
  BXVD_P_PPB_PullDown_eTopNextBottom   = 12,
  BXVD_P_PPB_PullDown_eBottomPrevTop   = 13,
  BXVD_P_PPB_PullDown_eBottomNextTop   = 14,
  BXVD_P_PPB_PullDown_eMax
} BXVD_P_PPB_PullDown;

/* Values for the 'frame_rate' field. */
typedef enum BXVD_P_PPB_FrameRate
{
  BXVD_P_PPB_FrameRate_eUnknown = 0,
  BXVD_P_PPB_FrameRate_e23_97,
  BXVD_P_PPB_FrameRate_e24,
  BXVD_P_PPB_FrameRate_e25,
  BXVD_P_PPB_FrameRate_e29_97,
  BXVD_P_PPB_FrameRate_e30,
  BXVD_P_PPB_FrameRate_e50,
  BXVD_P_PPB_FrameRate_e59_94,
  BXVD_P_PPB_FrameRate_e60,
  BXVD_P_PPB_FrameRate_e14_985,
  BXVD_P_PPB_FrameRate_e7_493,
  BXVD_P_PPB_FrameRate_e15,
  BXVD_P_PPB_FrameRate_e10,
  BXVD_P_PPB_FrameRate_e20,
  BXVD_P_PPB_FrameRate_e12_5, /* SW7445-2044: currently 10, 20 and 12.5 will not be delivered
                               * by AVD.  They were added for test, i.e use in the PPBReceived
                               * callback.  If added by AVD, need to sync this file and shared.h */
  BXVD_P_PPB_FrameRate_e119_88,
  BXVD_P_PPB_FrameRate_e120,
  BXVD_P_PPB_FrameRate_e100,
  BXVD_P_PPB_FrameRate_e19_98,
  BXVD_P_PPB_FrameRate_e7_5,
  BXVD_P_PPB_FrameRate_e12,
  BXVD_P_PPB_FrameRate_e11_988,
  BXVD_P_PPB_FrameRate_e9_99,
  BXVD_P_PPB_FrameRate_eMax

} BXVD_P_PPB_FrameRate;

/* Values for the 'matrix_coeff' field. */
typedef enum BXVD_P_PPB_MatrixCoeff
{
  BXVD_P_PPB_MatrixCoeff_eUnknown   = 0,
  BXVD_P_PPB_MatrixCoeff_eBT709,
  BXVD_P_PPB_MatrixCoeff_eUnspecified,
  BXVD_P_PPB_MatrixCoeff_eReserved,
  BXVD_P_PPB_MatrixCoeff_eFCC       = 4,
  BXVD_P_PPB_MatrixCoeff_eBT740_2BG,
  BXVD_P_PPB_MatrixCoeff_eSMPTE170M,
  BXVD_P_PPB_MatrixCoeff_eSMPTE240M,
  BXVD_P_PPB_MatrixCoeff_eSMPTE293M,
  BXVD_P_PPB_MatrixCoeff_eItu_R_BT_2020_NCL,
  BXVD_P_PPB_MatrixCoeff_eItu_R_BT_2020_CL
} BXVD_P_PPB_MatrixCoeff;

/* Values for the 'aspect_ratio' field. */
typedef enum BXVD_P_PPB_AspectRatio
{
  BXVD_P_PPB_AspectRatio_eUnknown = 0,
  BXVD_P_PPB_AspectRatio_eSquare,
  BXVD_P_PPB_AspectRatio_e12_11,
  BXVD_P_PPB_AspectRatio_e10_11,
  BXVD_P_PPB_AspectRatio_e16_11,
  BXVD_P_PPB_AspectRatio_e40_33,
  BXVD_P_PPB_AspectRatio_e24_11,
  BXVD_P_PPB_AspectRatio_e20_11,
  BXVD_P_PPB_AspectRatio_e32_11,
  BXVD_P_PPB_AspectRatio_e80_33,
  BXVD_P_PPB_AspectRatio_e18_11,
  BXVD_P_PPB_AspectRatio_e15_11,
  BXVD_P_PPB_AspectRatio_e64_33,
  BXVD_P_PPB_AspectRatio_e160_99,
  BXVD_P_PPB_AspectRatio_e4_3,
  BXVD_P_PPB_AspectRatio_e3_2,
  BXVD_P_PPB_AspectRatio_e2_1,
  BXVD_P_PPB_AspectRatio_e16_9,
  BXVD_P_PPB_AspectRatio_e221_1,
  BXVD_P_PPB_AspectRatio_eOther = 255
} BXVD_P_PPB_AspectRatio;

/* Values for the 'colour_primaries' field. */
typedef enum BXVD_P_PPB_ColorPrimaries
{
  BXVD_P_PPB_ColorPrimaries_eUnknown = 0,
  BXVD_P_PPB_ColorPrimaries_eBT709,
  BXVD_P_PPB_ColorPrimaries_eUnspecified,
  BXVD_P_PPB_ColorPrimaries_eReserved,
  BXVD_P_PPB_ColorPrimaries_eBT470_2M = 4,
  BXVD_P_PPB_ColorPrimaries_eBT470_2BG,
  BXVD_P_PPB_ColorPrimaries_eSMPTE170M,
  BXVD_P_PPB_ColorPrimaries_eSMPTE240M,
  BXVD_P_PPB_ColorPrimaries_eGenericFilm,
  BXVD_P_PPB_ColorPrimaries_eItu_R_BT_2020
} BXVD_P_PPB_ColorPrimaries;

/* Values for the 'transfer_char' field. */
typedef enum BXVD_P_PPB_TransferChar
{
  BXVD_P_PPB_TransferChar_eUnknown = 0,
  BXVD_P_PPB_TransferChar_eBT709,
  BXVD_P_PPB_TransferChar_eUnspecified,
  BXVD_P_PPB_TransferChar_eReserved,
  BXVD_P_PPB_TransferChar_eBT479_2M = 4,
  BXVD_P_PPB_TransferChar_eBT479_2BG,
  BXVD_P_PPB_TransferChar_eSMPTE170M,
  BXVD_P_PPB_TransferChar_eSMPTE240M,
  BXVD_P_PPB_TransferChar_eLinear,
  BXVD_P_PPB_TransferChar_eLog100_1,
  BXVD_P_PPB_TransferChar_eLog31622777_1,
  BXVD_P_PPB_TransferChar_eIec_61966_2_4,       /* 11 */
  BXVD_P_PPB_TransferChar_eReserved_2,          /* 12 */
  BXVD_P_PPB_TransferChar_eReserved_3,          /* 13 */
  BXVD_P_PPB_TransferChar_eItu_R_BT_2020_10bit, /* 14 */
  BXVD_P_PPB_TransferChar_eItu_R_BT_2020_12bit, /* 15 */
  BXVD_P_PPB_TransferChar_eSMPTE_ST_2084,       /* 16 */
  BXVD_P_PPB_TransferChar_eReserved_4,          /* 17 */
  BXVD_P_PPB_TransferChar_eARIB_STD_B67         /* 18 */
} BXVD_P_PPB_TransferChar;

/* SW7425-3177: Values for 'protocol' field.  These needed to be added back
 * in since the enums defining the video protocol have diverged between AVD
 * and bavc.h, i.e. BXVD_P_PPB_Protocol is no longer in sync with BAVC_VideoCompressionStd.
 */
typedef enum BXVD_P_PPB_Protocol
{
   BXVD_P_PPB_Protocol_eH264 = 0,
   BXVD_P_PPB_Protocol_eMPEG2,
   BXVD_P_PPB_Protocol_eH261,
   BXVD_P_PPB_Protocol_eH263,
   BXVD_P_PPB_Protocol_eVC1,
   BXVD_P_PPB_Protocol_eMPEG1,
   BXVD_P_PPB_Protocol_eMPEG2DTV,
   BXVD_P_PPB_Protocol_eVC1SimpleMain,
   BXVD_P_PPB_Protocol_eMPEG4Part2,
   BXVD_P_PPB_Protocol_eAVS,
   BXVD_P_PPB_Protocol_eMPEG2_DSS_PES,
   BXVD_P_PPB_Protocol_eSVC,            /* Scalable Video Codec */
   BXVD_P_PPB_Protocol_eSVC_BL,         /* Scalable Video Codec Base Layer */
   BXVD_P_PPB_Protocol_eMVC,            /* MVC Multi View Coding */
   BXVD_P_PPB_Protocol_eVP6,            /* VP6 */
   BXVD_P_PPB_Protocol_eVP7,            /* VP7 */
   BXVD_P_PPB_Protocol_eVP8,            /* VP8 */
   BXVD_P_PPB_Protocol_eRV9,            /* Real Video 9 */
   BXVD_P_PPB_Protocol_eSPARK,          /* Sorenson Spark */
   BXVD_P_PPB_Protocol_eH265,           /* HEVC */
   BXVD_P_PPB_Protocol_eVP9,            /* VP9 */

   BXVD_P_PPB_Protocol_eMax
} BXVD_P_PPB_Protocol;


/* Values for extracting the "protocol level" field from the PPB.
 * The "level" is the lower 16 bits.
 */
#define BXVD_P_PPB_PROTOCOL_LEVEL_MASK            0x0000FFFF
#define BXVD_P_PPB_PROTOCOL_LEVEL_SHIFT           0x0

/* Values for extracting the "protocol profile" field from the PPB.
 * The "profile" is the upper 16 bits.
 */
#define BXVD_P_PPB_PROTOCOL_PROFILE_MASK          0xFFFF0000
#define BXVD_P_PPB_PROTOCOL_PROFILE_SHIFT         0x10


#define BXVD_P_PPB_DNR_FLAG_REFPIC        0x01
#define BXVD_P_PPB_DNR_FLAG_GOPHEADER         0x02

typedef struct
{
   uint32_t      pic_flags;            /* Bit 2: GOP header seen. Bit 1: Sequence header seen Bit 0: reference pic  */
   uint32_t      sum_qp_cur;           /* sum of the quant params for this frame, or first field if interlaced */
   uint32_t      sum_qp_ref;           /* sum of the quant params for the reference pictures */
   uint32_t      num_mb_cur;           /* number of macroblocks included in sum_qp_cur */
   uint32_t      mb_count_intra_inter; /* number of  macroblocks in this picture: [31:16]=intra, [15:0]=non_intra */
   uint32_t      intra_qmat_sum_lo_hi; /* sum of intra quant matrix: [31:16]=first 36 entries, [15:0]=last 32 entries */
   uint32_t      inter_qmat_sum_lo_hi; /* sum of inter quant matrix: [31:16]=first 36 entries, [15:0]=last 32 entries */
   uint32_t      num_mb_ref;           /* number of macroblocks included in sum_qp_ref */
} BXVD_P_PPB_DNR;

/* data structure with key statistics collected by inner-loop for a picture */
/* some are available only for newer AVD core revisions */
typedef struct
{
   /* Number of cycles taken just by the inner-loop decode.
    * Does not include cabac or outer-loop processing times. */
   uint32_t      decode_cycle_count;

   /* the following are used/available for Rev J and later which
    * has a mocomp cache in there. will be 0 for earlier revisions. */
   uint32_t      pcache_hit;
   uint32_t      pcache_miss_1;
   uint32_t      pcache_miss_2;

   uint32_t      cabac_cylces;
   uint32_t      inst_cache_miss;
} BXVD_P_PPB_IL_DATA;

#if BXVD_P_PPB_EXTENDED

typedef struct
{
    uint32_t      crc_luma;            /* frame or top field of LUMA CRC */
    uint32_t      crc_cb;              /* frame or top field of CHROMA CB CRC */
    uint32_t      crc_cr;              /* frame or top field of CHROMA CR CRC */
    uint32_t      crc_luma_bot;        /* bot field of LUMA CRC */
    uint32_t      crc_cb_bot;          /* bot field of CHROMA CB CRC */
    uint32_t      crc_cr_bot;          /* bot field of CHROMA CR CRC */
} BXVD_P_PPB_CRC;

/* Data structure with key statistics collected by outer-loop for a frame. */
/* For interlaced data,  statistics are accummulated over both fields-     */
/* statistics per field would go over 256 byte limit  */

typedef struct
{
    uint32_t      idle_time;                  /* IL idle time; stall time while waiting for IL decode command */
    unsigned short    num_decod_mbs;              /* (H264 only) number of macroblock are decoded */
    unsigned short    num_recov_mbs;              /* (H264 only) Number Of Recovered Macroblocks: number of macroblocks which where error corrected */
    uint32_t      totalDecodeCycles;          /* total IL frame  decode cycle  */
    uint32_t      totalCabacCycles;           /* total CABAC frame cycle for pair field picture */
    uint32_t      totalILBLDecodeCycles;      /* svc IL frame decode cycle count include wait for BL */
    uint32_t      totalFrm2FrmDecodeCycles;   /* IL frame cycle count from frame to frame (include wit for (BL), OL) */
    uint32_t      reserved[4];
} BXVD_P_PPB_IL_LOG_DATA;


typedef struct
{
    uint32_t      decode_cycle_count;
    uint32_t      inst_cache_miss;
    uint32_t      display_time;
    uint32_t      cdb_size;
    uint32_t      picture_latency;
    uint32_t      cache_misses;
    uint32_t      reserved[2];
} BXVD_P_PPB_OL_LOG_DATA;

typedef struct
{
    BXVD_P_PPB_IL_LOG_DATA il_log_data;
    BXVD_P_PPB_OL_LOG_DATA ol_log_data;
} BXVD_P_PPB_LOG_DATA;

typedef struct
{
    unsigned int client_read;
    unsigned int CAS;
    unsigned int intra_penalty;
    unsigned int post_penalty;

} BXVD_P_PPB_DRAM_PERF;

#endif /* #if BXVD_P_PPB_EXTENDED */

typedef struct
{
   /* Common fields. */
   uint32_t      flags;            /* see above         */
   uint32_t      luma_video_address;    /* Address of picbuf Y     */
#if BXVD_P_CORE_40BIT_ADDRESSABLE
   uint32_t      luma_video_address_hi;    /* Address of picbuf Y     */
#endif
   uint32_t      chroma_video_address; /* Address of picbuf UV    */
#if BXVD_P_CORE_40BIT_ADDRESSABLE
   uint32_t      chroma_video_address_hi; /* Address of picbuf UV    */
#endif
   uint32_t      video_width;      /* Picbuf width      */
   uint32_t      video_height;     /* Picbuf height     */

   uint32_t      luma_stripe_height;
   uint32_t      chroma_stripe_height;
   uint32_t      pulldown;         /* see above         */
   uint32_t      protocol;         /* protocolXXX (above)     */

   uint32_t      frame_rate;       /* see above         */
   uint32_t      aspect_ratio;     /* see above         */
   uint32_t      custom_aspect_ratio_width_height; /* upper 16-bits is Y and lower 16-bits is X */
   uint32_t      display_info;     /* [31:24]=matrix_coeff, [23:16]=color_primaries, [15:8]=transfer_char */

   uint32_t      pts;              /* 32 LSBs of PTS    */
   uint32_t      pcr_offset;       /* 45kHz if PCR type; else 27MHz */
   uint32_t      n_drop;           /* Number of pictures to be dropped */

   /* User data is in the form of a linked list. */
   uint32_t          user_data_size;
   uint32_t          user_data;

   /* DNR parameters */
   BXVD_P_PPB_DNR     dnr;              /* DNR parameters for entire frame */

   uint32_t      timing_marker;
   uint32_t      picture_tag;      /* tag used during on-the-fly PVR, similar to PTS */

   /* SW7425-2247: support for bar data. This field was
    * formerly fgt_block_avg_address
    */
   uint32_t      bar_data;
   uint32_t      profile_level;

   uint32_t      afd_val;         /* Active Format Descriptor */

   /* IL decode information */
   BXVD_P_PPB_IL_DATA       il_perf_data;

   uint32_t      delta_pic_seen;

   /* W7401-4426: H264 time code [24:20]=hour, [19:14]=minute, [13:8]=sec, [7:0]=frame */
   uint32_t     time_code;

   /* SWSTB-439: the ratio of macro blocks with an error. 8 bit value with a range of 0 to 255. */
   uint32_t     error_level;

   /* SW7425-2686: multi-pass DQT, number of picture buffers that can be outstanding. */
   uint32_t     num_pic_buffers;


   /* Protocol-specific extensions. */
   union
   {
      BXVD_P_PPB_H264        h264;
      BXVD_P_PPB_H265        h265;
      BXVD_P_PPB_SVC         svc;
      BXVD_P_PPB_MPEG        mpeg;
      BXVD_P_PPB_VC1         vc1;
      BXVD_P_PPB_AVS         avs;
      BXVD_P_PPB_MP4         mp4;
      BXVD_P_PPB_VP8         vp8;
      BXVD_P_PPB_RV9         rv9;
      BXVD_P_PPB_VP9         vp9;

#ifndef BXVD_P_PPB_EXTENDED

   } other;

#else
      uint32_t             stub[32];   /* to force this union to be a minimum of 32 words. */

   } other;

   BXVD_P_PPB_CRC          crc;
   uint32_t                flags_ext0;

   /* SW7445-1954: [15:8] chroma bit width, [7:0]: luma bit width.
    * Only valid when using luma/chroma buffers > 8 bits.  */
   uint32_t                luma_chroma_bit_width;

   uint32_t                ppb_reserved[29];
   union
   {
      BXVD_P_PPB_LOG_DATA  data_log;
      BXVD_P_PPB_DRAM_PERF dram_perf;
   } perf;

#endif

} BXVD_P_PPB;


/* Display Manager: STC and Parity Information in DRAM for host to use */

#if BXVD_P_FW_HIM_API

#if BXVD_P_HVD_PRESENT
/* Hercules */
#if BXVD_P_DECODER_REVP
/* Will be 4 for new FW memory config */
#define  BXVD_P_CURRENT_MAJOR_VERSION 4

#elif BXVD_P_DECODER_REVN1
#define BXVD_P_CURRENT_MAJOR_VERSION 3

#elif BXVD_P_DECODER_REVT
#define  BXVD_P_CURRENT_MAJOR_VERSION 1

#else
#define BXVD_P_CURRENT_MAJOR_VERSION 2
#endif

#define BXVD_P_STC_MAX 16

#else
/* Aphrodite */
#define BXVD_P_CURRENT_MAJOR_VERSION 5
#define BXVD_P_STC_MAX 8
#endif



typedef struct
{
    uint32_t stc_snapshot[BXVD_P_STC_MAX];
    uint32_t vsync_parity;
#if BXVD_P_HVD_PRESENT
    uint32_t vsync_parity_1;
#endif
} BXVD_P_DisplayInfo;

#else

#define BXVD_P_CURRENT_MAJOR_VERSION 3

#define BXVD_P_STC_MAX 2

typedef struct
{
      uint32_t stc_snapshot;
      uint32_t vsync_parity;
      uint32_t stc1_snapshot;
} BXVD_P_DisplayInfo;

#endif   /* ~BXVD_P_FW_HIM_API */


#if BXVD_P_FW_HIM_API
#define BXVD_P_MAX_ELEMENTS_IN_DISPLAY_QUEUE              64
#define BXVD_P_INITIAL_OFFSET_DISPLAY_QUEUE                0
#else
#define BXVD_P_MAX_ELEMENTS_IN_DISPLAY_QUEUE              62
#define BXVD_P_INITIAL_OFFSET_DISPLAY_QUEUE                2
#endif

#define BXVD_P_DISP_FIFO_DEPTH   ( BXVD_P_MAX_ELEMENTS_IN_DISPLAY_QUEUE + BXVD_P_INITIAL_OFFSET_DISPLAY_QUEUE )

#if 0
/* Parameters passed from Display Manager (host) to DMS */
typedef struct
{
   uint32_t write_offset;
   uint32_t drop_count;
} BXVD_P_DMS_Info;
#endif

typedef struct
{

#if !BXVD_P_FW_HIM_API
    uint32_t queue_read_offset; /* offset is w.r.t base of this data struct so value of 0-1 prohibited */
    uint32_t queue_write_offset; /* offset is w.r.t base of this data struct so value of 0-1 prohibited */
#endif

    /* queue if full if (write_offset+1 == read_offset) */
    /* write_offset modified by firmware and read_offset modified by Display Manager in host */
    uint32_t display_elements[BXVD_P_MAX_ELEMENTS_IN_DISPLAY_QUEUE];

} BXVD_P_Avd_Queue_In_Memory;

#define BXVD_P_PictureDeliveryQueue BXVD_P_Avd_Queue_In_Memory
#define BXVD_P_PictureReleaseQueue BXVD_P_Avd_Queue_In_Memory

/* picture information block (PIB) */
typedef struct BXVD_P_PPB_PIB
{
    BXVD_P_PPB    sPPB;
    uint32_t      bFormatChange;
    uint32_t      ulResolution;
    uint32_t      ulChannelId;
    uint32_t      ppbPtr;
} BXVD_P_PPB_PIB;



/* The following two sections are mask and shift constant definitions based on FW team defined VDEC_FLAG above.
 * If the VDEC_FLAG bits change, these values will also need to change */

/* picture structure is in bits 0 & 1 */
#define BXVD_P_PPB_FLAG_EXTRACT_PICT_STRUCT_MASK         0x03
#define BXVD_P_PPB_FLAG_EXTRACT_PICT_STRUCT_SHIFT        0x0

/* progressive frame in bits 2 & 3 */
#define BXVD_P_PPB_FLAG_EXTRACT_SOURCE_FORMAT_MASK       0xc
#define BXVD_P_PPB_FLAG_EXTRACT_SOURCE_FORMAT_SHIFT      0x2

/* Paramaters passed from DMS to DM */
typedef struct BXVD_P_DMS2DMInfo
{
    BXVD_P_DisplayInfo *pDisplayInfo ;
    BXVD_P_PictureDeliveryQueue *pPictureDeliveryQueue ;
    BXVD_P_PictureReleaseQueue  *pPictureReleaseQueue ;
} BXVD_P_DMS2DMInfo ;

/* Parameters passed from DM to DMS */
typedef struct BXVD_P_DM2DMSInfo
{
   uint32_t pdq_write_offset ;
   uint32_t drop_count;
} BXVD_P_DM2DMSInfo ;

typedef struct BXVD_P_DisplayElement
{
    BXVD_P_PPB *pPPB ;
/*  BXVD_P_PPB *pPPBPhysical ; */
    uint32_t pPPBPhysical ;
    BAVC_Polarity ePPBPolarity;
} BXVD_P_DisplayElement ;

typedef struct BXVD_P_DisplayElementQueue
{
    BXVD_P_PPB *pPPBQArray[ BXVD_P_MAX_ELEMENTS_IN_DISPLAY_QUEUE ] ;
    uint32_t read_pointer ;
    uint32_t write_pointer ;
    uint32_t depth_pointer ;
} BXVD_P_DisplayElementQueue ;
/************************************************************************/
/* ABOVE ARE THE STRUCTURES ADDED BY SSAVEKAR FOR XVD INTEGRATION       */
/************************************************************************/

#endif /* __INC_VDEC_INFO_H__ */
