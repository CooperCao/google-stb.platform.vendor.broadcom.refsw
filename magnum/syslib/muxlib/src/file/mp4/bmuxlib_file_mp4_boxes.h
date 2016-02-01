/***************************************************************************
 *     Copyright (c) 2010-2012, Broadcom Corporation
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
 * Box processing definitions for File-based MP4 software mux
 *
 * [Revision History:]
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BMUXLIB_FILE_MP4_BOXES_H__
#define BMUXLIB_FILE_MP4_BOXES_H__

/* Includes */

#ifdef __cplusplus
extern "C" {
#endif

/****************************
*  D E F I N I T I O N S    *
****************************/

/* Four Character Codes in ASCII */
#define MP4MUX_MAKE_FOURCC(a,b,c,d) \
   ((a << 24) | (b << 16) | (c << 8) | d)

/********************
  Generic constants
********************/
/* NOTE: version is a byte field, so no conversion required */
#define BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0    0
#define BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1    1

#define BMUXLIB_FILE_MP4_P_NULL_CHAR            '\0'

/* this must be the index of the "trak" box in the box sequence table
   (this allows the state machine to repeat for other tracks) */
#define BMUXLIB_FILE_MP4_P_SEQUENCE_INDEX_TRAK  3

/* maximum mdat size that can be tolerated before a co64 box becomes necessary for
   offset storage.  Note, this assumes that the moov and other headers occupy no
   more than 5% of the total file - this is only likely to be untrue for small
   files, in which case it does not matter, since offsets will be << 32-bits */
#define BMUXLIB_FILE_MP4_P_MAX_MDAT_FOR_STCO    0xF3333333

/* MP4 descriptors have a maximum size of 2^28 -1 bytes */
#define BMUXLIB_FILE_MP4_P_MAX_DESC_SIZE        0x0FFFFFFF

/***************
  fullbox flags
***************/
#define BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE   0

/* flags for tkhd box ... */
#define BMUXLIB_FILE_MP4_P_TKHD_FLAG_ENABLED    0x000001
#define BMUXLIB_FILE_MP4_P_TKHD_FLAG_IN_MOVIE   0x000002
#define BMUXLIB_FILE_MP4_P_TKHD_FLAG_IN_PREVIEW 0x000004

/* flags for vmhd box ... */
#define BMUXLIB_FILE_MP4_P_VMHD_FLAG            0x000001

/* flags for url box ... */
#define BMUXLIB_FILE_MP4_P_URL_FLAG_MEDIA_IN_FILE  0x000001

/**************
   Brands
***************/

#define BMUXLIB_FILE_MP4_P_BRAND_ISOM           MP4MUX_MAKE_FOURCC('i','s','o','m')
#define BMUXLIB_FILE_MP4_P_BRAND_MP42           MP4MUX_MAKE_FOURCC('m','p','4','2')
#define BMUXLIB_FILE_MP4_P_BRAND_MP41           MP4MUX_MAKE_FOURCC('m','p','4','1')
#define BMUXLIB_FILE_MP4_P_BRAND_AVC1           MP4MUX_MAKE_FOURCC('a','v','c','1')

/**************
   Box Types
***************/

#define BMUXLIB_FILE_MP4_P_BOX_TYPE_FTYP        MP4MUX_MAKE_FOURCC('f','t','y','p')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_MOOV        MP4MUX_MAKE_FOURCC('m','o','o','v')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_MDAT        MP4MUX_MAKE_FOURCC('m','d','a','t')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_MVHD        MP4MUX_MAKE_FOURCC('m','v','h','d')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_TRAK        MP4MUX_MAKE_FOURCC('t','r','a','k')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_TKHD        MP4MUX_MAKE_FOURCC('t','k','h','d')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_EDTS        MP4MUX_MAKE_FOURCC('e','d','t','s')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_ELST        MP4MUX_MAKE_FOURCC('e','l','s','t')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_MDIA        MP4MUX_MAKE_FOURCC('m','d','i','a')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_MDHD        MP4MUX_MAKE_FOURCC('m','d','h','d')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_HDLR        MP4MUX_MAKE_FOURCC('h','d','l','r')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_MINF        MP4MUX_MAKE_FOURCC('m','i','n','f')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_VMHD        MP4MUX_MAKE_FOURCC('v','m','h','d')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_SMHD        MP4MUX_MAKE_FOURCC('s','m','h','d')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_HMHD        MP4MUX_MAKE_FOURCC('h','m','h','d')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_NMHD        MP4MUX_MAKE_FOURCC('n','m','h','d')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_DINF        MP4MUX_MAKE_FOURCC('d','i','n','f')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_DREF        MP4MUX_MAKE_FOURCC('d','r','e','f')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_URL         MP4MUX_MAKE_FOURCC('u','r','l',' ')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_STBL        MP4MUX_MAKE_FOURCC('s','t','b','l')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_STSD        MP4MUX_MAKE_FOURCC('s','t','s','d')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_STTS        MP4MUX_MAKE_FOURCC('s','t','t','s')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_CTTS        MP4MUX_MAKE_FOURCC('c','t','t','s')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_STSC        MP4MUX_MAKE_FOURCC('s','t','s','c')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_STSZ        MP4MUX_MAKE_FOURCC('s','t','s','z')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_STCO        MP4MUX_MAKE_FOURCC('s','t','c','o')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_CO64        MP4MUX_MAKE_FOURCC('c','o','6','4')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_STSS        MP4MUX_MAKE_FOURCC('s','t','s','s')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_AVC1        MP4MUX_MAKE_FOURCC('a','v','c','1')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_AVCC        MP4MUX_MAKE_FOURCC('a','v','c','C')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_AVCP        MP4MUX_MAKE_FOURCC('a','v','c','p')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_MP4A        MP4MUX_MAKE_FOURCC('m','p','4','a')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_MP4V        MP4MUX_MAKE_FOURCC('m','p','4','v')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_MP4S        MP4MUX_MAKE_FOURCC('m','p','4','s')
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_ESDS        MP4MUX_MAKE_FOURCC('e','s','d','s')

/****************
  Handler types
*****************/

#define BMUXLIB_FILE_MP4_P_HANDLER_VIDE         MP4MUX_MAKE_FOURCC('v','i','d','e')
#define BMUXLIB_FILE_MP4_P_HANDLER_SOUN         MP4MUX_MAKE_FOURCC('s','o','u','n')
#define BMUXLIB_FILE_MP4_P_HANDLER_HINT         MP4MUX_MAKE_FOURCC('h','i','n','t')
#define BMUXLIB_FILE_MP4_P_HANDLER_ODSM         MP4MUX_MAKE_FOURCC('o','d','s','m')
#define BMUXLIB_FILE_MP4_P_HANDLER_SDSM         MP4MUX_MAKE_FOURCC('s','d','s','m')

/************************
  MPEG 4 Descriptor Tags
************************/

#define BMUXLIB_FILE_MP4_P_DESC_ES              0x03
#define BMUXLIB_FILE_MP4_P_DESC_DEC_CONF        0x04
#define BMUXLIB_FILE_MP4_P_DESC_DEC_SPEC        0x05
#define BMUXLIB_FILE_MP4_P_DESC_SL_CONF         0x06

#define BMUXLIB_FILE_MP4_P_DESC_MP4_ES_ID_INC   0x0E
#define BMUXLIB_FILE_MP4_P_DESC_MP4_ES_ID_REF   0x0F
#define BMUXLIB_FILE_MP4_P_DESC_MP4_IOD         0x10
#define BMUXLIB_FILE_MP4_P_DESC_MP4_OD          0x11

/************************
  MPEG 4 Object Types
************************/

#define BMUXLIB_FILE_MP4_P_OBJ_TYPE_MPEG4_SYS   0x01
#define BMUXLIB_FILE_MP4_P_OBJ_TYPE_MPEG4_VID   0x20
#define BMUXLIB_FILE_MP4_P_OBJ_TYPE_AVC         0x21
#define BMUXLIB_FILE_MP4_P_OBJ_TYPE_MPEG4_AUD   0x40

/************************
  MPEG 4 Stream Types
************************/

#define BMUXLIB_FILE_MP4_P_STR_TYPE_ODS         0x01
#define BMUXLIB_FILE_MP4_P_STR_TYPE_SDS         0x03
#define BMUXLIB_FILE_MP4_P_STR_TYPE_VIS         0x04
#define BMUXLIB_FILE_MP4_P_STR_TYPE_AUD         0x05

/************************
   Box Field Constants
************************/

/* all sample sizes in the stsz are different */
#define BMUXLIB_FILE_MP4_P_SAMPLE_SIZE_ALL_DIFF    0

/* rate of 1.0 in 16.16 fixed point */
#define BMUXLIB_FILE_MP4_P_RATE_1_0                0x00010000

/* volume of 1.0 in 8.8 fixed point */
#define BMUXLIB_FILE_MP4_P_VOLUME_1_0              0x0100

#define BMUXLIB_FILE_MP4_P_RESERVED                0  /* MUST have the value zero */

#define BMUXLIB_FILE_MP4_P_FTYP_MINOR_VER          0

/* ISO 639-2 language code
   we use "und" (undetermined) for all content.
   This is a packed value of 3, 5-bit values that
   are the diff of the ASCII char from 0x60
   "u" = 0x75 => 0x15 => 10101
   "n" = 0x6E => 0x0E => 01110
   "d" = 0x64 => 0x04 => 00100
   Overall code is 0 10101 01110 00100 */
#define BMUXLIB_FILE_MP4_P_LANGUAGE_CODE_UND       0x55C4

/* constant to indicate an unbounded box (size = 0) */
#define BMUXLIB_FILE_MP4_P_BOX_SIZE_UNBOUNDED      0

/* constant to indicate a largesize box (indicates presence of largesize field) */
#define BMUXLIB_FILE_MP4_P_BOX_SIZE_LARGESIZE      1

/* video sequence depth (24 bits, no alpha) */
#define BMUXLIB_FILE_MP4_P_VIDEO_DEPTH             0x0018

/* video sequence resolution = 72dpi (16.16 number) */
#define BMUXLIB_FILE_MP4_P_VIDEO_RESOLUTION        0x00480000

/* Video frame count (per sample) = 1 */
#define BMUXLIB_FILE_MP4_P_VIDEO_FRAME_COUNT       1

/* fixed size compression name field length */
#define BMUXLIB_FILE_MP4_P_VIDEO_COMP_NAME_LEN     32

/* pre-defined field in video sample entry */
#define BMUXLIB_FILE_MP4_P_VIDEO_PREDEFINED        -1

/* audio sequence channel count */
#define BMUXLIB_FILE_MP4_P_AUDIO_CHANNEL_COUNT     2

/* audio sequence sample size (16 bits) */
#define BMUXLIB_FILE_MP4_P_AUDIO_SAMPLE_SIZE       16

/* avcC fields (byte values) */
#define BMUXLIB_FILE_MP4_P_AVCC_RESERVED_3BITS     0xE0

#define BMUXLIB_FILE_MP4_P_AVCC_RESERVED_6BITS     0xFC

#define BMUXLIB_FILE_MP4_P_AVCC_CONFIG_VER         0x01

/* The following has a maximum size of 5 bits */
#define BMUXLIB_FILE_MP4_P_AVCC_NUM_SEQ_PARAM_SETS 1

#define BMUXLIB_FILE_MP4_P_AVCC_NUM_PIC_PARAM_SETS 1

/* NALU length is always 4 bytes in size
(this field has a max size of 5 bits) */
#define BMUXLIB_FILE_MP4_P_AVCC_NALU_LENGTH_SIZE_MINUS_1 0x3

#define BMUXLIB_FILE_MP4_P_DESC_DECCONF_UPSTREAM   0x00

#define BMUXLIB_FILE_MP4_P_DESC_DECCONF_RES_1BIT   0x01

/* ES Descriptor flags: stream dependence, URL, OCR stream, stream priority
   - all zero */
#define BMUXLIB_FILE_MP4_P_ESD_FLAGS               0

/* edit list media time entry for an empty edit
   (used for specifying starting offsets) */
#define BMUXLIB_FILE_MP4_P_EMPTY_EDIT              -1

/******************************
  Descriptor Field Constants
******************************/

#define BMUXLIB_FILE_MP4_P_DESC_SIZE_BITS          7
#define BMUXLIB_FILE_MP4_P_DESC_SIZE_MASK          0x7F
/* top bit in each byte of the size indicates extension to another byte */
#define BMUXLIB_FILE_MP4_P_DESC_SIZE_EXT_BIT       0x80

/* SL Config Descriptor requires use of pre-defined = 2 */
#define BMUXLIB_FILE_MP4_P_SL_CONF_PRE_DEF         2

/*********************
   Box Field Sizes
*********************/

#define BMUXLIB_FILE_MP4_P_BOX_SIZE_FIELD_SIZE           sizeof(uint32_t)
#define BMUXLIB_FILE_MP4_P_BOX_TYPE_FIELD_SIZE           sizeof(uint32_t)
#define BMUXLIB_FILE_MP4_P_BOX_LARGESIZE_FIELD_SIZE      sizeof(uint64_t)
#define BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_FLAGS_SIZE    sizeof(uint32_t)

#define BMUXLIB_FILE_MP4_P_BOX_HEADER_SIZE               (BMUXLIB_FILE_MP4_P_BOX_SIZE_FIELD_SIZE + BMUXLIB_FILE_MP4_P_BOX_TYPE_FIELD_SIZE)
#define BMUXLIB_FILE_MP4_P_FULLBOX_HEADER_SIZE           (BMUXLIB_FILE_MP4_P_BOX_SIZE_FIELD_SIZE + \
                                                          BMUXLIB_FILE_MP4_P_BOX_TYPE_FIELD_SIZE + \
                                                          BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_FLAGS_SIZE)

/*********************
   Fixed Box PL Sizes
**********************/
/* fixed-size boxes - does not include boxtype or size fields, or version/flags of a fullbox
   (the sizes of the fields boxtype, size and version/flags is taken care of when the box is created) */

/* used for boxes where the PL size is not known in advance (an unbounded box)
   - size is determined when box is complete and updated in-place in the output */
#define BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN                0

/* the following is used to distinguish boxes where we know there is no content
   from boxes that are of an unknown size when written */
#define BMUXLIB_FILE_MP4_P_PLSIZE_EMPTY                  0

/* ftyp PL size = major brand (4 bytes) + minor version (4 bytes) + compatibility brands * 3 (4 * 3) = 20 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_FTYP                   20

/* mvhd PL size (version 0) = create_time (4 bytes) + mod_time (4 bytes) + timescale (4 bytes) + duration (4 bytes)
      + rate (4 bytes) + volume (2) + reserved (10) + matrx (36) + pre-defined (24) + next_track_id (4 bytes)
      = 96 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_MVHD_0                 96

/* mvhd PL size (version 1) = same as above except create_time, mod_time and duration are 64-bit
      = 96 + (3 * 4) = 108 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_MVHD_1                 108

/* tkhd PL size (version 0) = create_time (4 bytes) + mod_time (4) + track ID (4) + reserved (4) + duration (4)
      + reserved (2 * 4) + layer (2) + alt_group (2) + volume (2) + reserved (2) + matrix (36) + width (4) + height (4)
      = 44 + 36 = 80 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_TKHD_0                 80

/* tkhd PL size (version 1) = same as above except create_time, mod_time and duration are 64-bit
      = 80 + (3 * 4) = 92 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_TKHD_1                 92

/* elst PL size (version 0) = entry count (4 bytes) + N * (segduration (4 bytes) + media time (4 bytes) + media rate (4 bytes))
   (for now, only two entries defined (N = 2) (offset + segment)) */
#define BMUXLIB_FILE_MP4_P_PLSIZE_ELST_0                 28

/* elst PL size (version 1) = as above, except segduration & media time are 64 bit */
#define BMUXLIB_FILE_MP4_P_PLSIZE_ELST_1                 44

/* edts PL size = size of elst box = elst PL size + fullbox headers */
#define BMUXLIB_FILE_MP4_P_PLSIZE_EDTS_0                 (BMUXLIB_FILE_MP4_P_PLSIZE_ELST_0 + BMUXLIB_FILE_MP4_P_FULLBOX_HEADER_SIZE)
#define BMUXLIB_FILE_MP4_P_PLSIZE_EDTS_1                 (BMUXLIB_FILE_MP4_P_PLSIZE_ELST_1 + BMUXLIB_FILE_MP4_P_FULLBOX_HEADER_SIZE)

/* mdhd PL size (version 0) = create_time (4 bytes) + mod_time (4) + timescale (4) + duration (4) +
       language code (2) + pre-defined (2)
      = 20 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_MDHD_0                 20

/* mdhd PL size (version 1) = same as above except create_time, mod_time and duration are 64-bit
      = 20 + (3 * 4) = 32 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_MDHD_1                 32

/* hdlr PL size = pre-defined (4) + handler_type (4) + reserved (3 * 4) + name (1)
      = 21 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_HDLR                   21

/* vmhd PL size = graphics mode (2) + opcolor (3 * 2)
      = 8 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_VMHD                   8

/* smhd PL size = balance (2) + reserved (2)
      = 4 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_SMHD                   4

/* hmhd PL size = maxPDUSize (2) + avgPDUsize (2) + maxbitrate (4) + avgbitrate (4) + reserved (4)
      = 16 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_HMHD                   16

/* dref PL size = entry_count (4) + size of an empty fullbox (the url box) */
#define BMUXLIB_FILE_MP4_P_PLSIZE_DREF                   (4 + BMUXLIB_FILE_MP4_P_FULLBOX_HEADER_SIZE)

/* dinf PL size = size of dref box = dref PL size + fullbox headers */
#define BMUXLIB_FILE_MP4_P_PLSIZE_DINF                   (BMUXLIB_FILE_MP4_P_PLSIZE_DREF + BMUXLIB_FILE_MP4_P_FULLBOX_HEADER_SIZE)

/* url PL size = 0 (no data allowed in the URL boxsince we set the "media in file" flag) */
#define BMUXLIB_FILE_MP4_P_PLSIZE_URL                    0

/* NOTE: avcC PL size does not include the parameter set NALUs - these sizes are accounted for separately
   avcC PL size = config ver (1) + profile (1) + flags (1) + level (1) + NALU length size (1) + num SPS (1) + SPS size (2)
        + num PPS (1) + PPS size (2)
       = 11 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_AVCC                   11

/* size of the video sample entry, not including the parts contributed by the sample entry
   that is extended by this:
    PL size = pre-defined/reserved (16) + width (2) + height (2) + horiz res (4) + vert res (4) + reserved (4)
       + frame count (2) + compressor name (32) + depth (2) + predefined (2)
       = 70 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_VIDEO_SAMPLE_ENTRY     70

/* size of the audio sample entry, not including the parts contributed by the sample entry
   that is extended by this:
    PL size = reserved (8) + channels (2) + sample size (2) + pre-defined/reserved (4) + sample rate (4)
       = 20 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_AUDIO_SAMPLE_ENTRY     20

/* sample entry PL size = reserved (6) + data_reference_index (2)
      = 8 bytes */
#define BMUXLIB_FILE_MP4_P_PLSIZE_SAMPLE_ENTRY           8

/* esds PL size = size of es descriptor */
#define BMUXLIB_FILE_MP4_P_PLSIZE_ESDS                   BMUXLIB_FILE_MP4_P_DESC_SIZE_ES

/* stsc PL size = entry_count (4 bytes) + N * (first chunk (4) + samp per chunk (4) + samp desc idx (4))
   for our purposes, this is a fixed box with only one entry (N=1) */
#define BMUXLIB_FILE_MP4_P_PLSIZE_STSC                   16

/* stsz PL size = sample_size (4 bytes) + sample count (4) + N * entry_size (4)
   Note that the entry sizes are separately filled in by metadata processing so are not
   treated as part of the box payload (unbounded box) */
#define BMUXLIB_FILE_MP4_P_PLSIZE_STSZ                   8


/**********************************
   MPEG 4 Descriptor Field Sizes
**********************************/

#define BMUXLIB_FILE_MP4_P_DESC_TAG_SIZE                 1
/* length field is always the extended 4-byte length ... */
#define BMUXLIB_FILE_MP4_P_DESC_LENGTH_FIELD_SIZE        4

#define BMUXLIB_FILE_MP4_P_DESC_HEADER_SIZE              (BMUXLIB_FILE_MP4_P_DESC_TAG_SIZE + BMUXLIB_FILE_MP4_P_DESC_LENGTH_FIELD_SIZE)

/**********************************
   MPEG 4 Descriptor PL Sizes
**********************************/

/* size of ESD (not including the sizes contributed by the Dec Config or SL Config descriptors)
      = ES ID (2) + prio/flags (1)
      = 3 bytes */
#define BMUXLIB_FILE_MP4_P_DESC_PLSIZE_ESD                 3

/* size of Dec Config (not including size contributed by the dec specific info)
      = obj type (1) + stream type (1) + buffer size (3) + max bitrate (4) + avg bitrate (4)
      = 13 bytes */
#define BMUXLIB_FILE_MP4_P_DESC_PLSIZE_DEC_CONFIG          13

/* size of SL Config is one byte (predefined) */
#define BMUXLIB_FILE_MP4_P_DESC_PLSIZE_SL_CONFIG           1

/***************************
  MPEG 4 Descriptor Sizes
***************************/
/* (includes all header bytes) */

/* size of Desc Spec Info Descriptor = size of desc header + size of dec specific info */
#define BMUXLIB_FILE_MP4_P_DESC_SIZE_DEC_SPEC            (BMUXLIB_FILE_MP4_P_DESC_HEADER_SIZE + \
                                                            pTrack->uiDecSpecInfoSize)

/* size of SL Config descriptor = size of desc header + size of SL Config Payload */
#define BMUXLIB_FILE_MP4_P_DESC_SIZE_SL_CONF             (BMUXLIB_FILE_MP4_P_DESC_HEADER_SIZE + \
                                                            BMUXLIB_FILE_MP4_P_DESC_PLSIZE_SL_CONFIG)

/* size of Dec Config Descriptor = size of desc header + size of Dec Config Payload
      + size of Dec Spec Info descriptor */
#define BMUXLIB_FILE_MP4_P_DESC_SIZE_DEC_CONF            (BMUXLIB_FILE_MP4_P_DESC_HEADER_SIZE + \
                                                            BMUXLIB_FILE_MP4_P_DESC_PLSIZE_DEC_CONFIG + \
                                                            BMUXLIB_FILE_MP4_P_DESC_SIZE_DEC_SPEC)

/* size of ES descriptor = size of Desc Header +  size of ESD payload
      + size of Decoder Config Descriptor + size of SL Config Descriptor */
#define BMUXLIB_FILE_MP4_P_DESC_SIZE_ES                  (BMUXLIB_FILE_MP4_P_DESC_HEADER_SIZE + \
                                                            BMUXLIB_FILE_MP4_P_DESC_PLSIZE_ESD + \
                                                            BMUXLIB_FILE_MP4_P_DESC_SIZE_DEC_CONF + \
                                                            BMUXLIB_FILE_MP4_P_DESC_SIZE_SL_CONF)

/****************************
*        T Y P E S          *
****************************/

/* transformation matrix:
   [a  b  u]
   [c  d  v]
   [x  y  w]
*/
typedef struct
{
   uint32_t a; /* 16.16 fixed */
   uint32_t b; /* 16.16 fixed */
   uint32_t u; /* 2.30 fixed */
   uint32_t c; /* 16.16 fixed */
   uint32_t d; /* 16.16 fixed */
   uint32_t v; /* 2.30 fixed */
   uint32_t x; /* 16.16 fixed */
   uint32_t y; /* 16.16 fixed */
   uint32_t w; /* 2.30 fixed */
} BMUXlib_File_MP4_P_Matrix;

/* all these boxes are unbounded boxes, EXCEPT ftyp
   - i.e. their sizes are not fixed, and not known when the box is created;
   fixed size boxes are incorporated into the processing of other unbounded boxes
   (e.g. mvhd is created when the moov is created) */
typedef enum
{
   BMUXlib_File_MP4_P_BoxType_eUnknown,
   BMUXlib_File_MP4_P_BoxType_eFtyp,
   BMUXlib_File_MP4_P_BoxType_eMdat,
   BMUXlib_File_MP4_P_BoxType_eMoov,      /* moov/mvhd */
   BMUXlib_File_MP4_P_BoxType_eTrak,      /* trak/tkhd */
   BMUXlib_File_MP4_P_BoxType_eMdia,      /* mdia/mdhd/hdlr */
   BMUXlib_File_MP4_P_BoxType_eMinf,      /* minf/vmhd/smhd/hmhd/nmhd/dinf/dref */
   BMUXlib_File_MP4_P_BoxType_eStbl,
   BMUXlib_File_MP4_P_BoxType_eStsd,
   BMUXlib_File_MP4_P_BoxType_eStts,
   BMUXlib_File_MP4_P_BoxType_eCtts,
   BMUXlib_File_MP4_P_BoxType_eStsc,
   BMUXlib_File_MP4_P_BoxType_eStsz,
   BMUXlib_File_MP4_P_BoxType_eStco,
   BMUXlib_File_MP4_P_BoxType_eStss,
   /* ... ADD other boxes here ... */
   /* e.g. moof/mfhd, traf/tfhd, trun, etc */
   BMUXlib_File_MP4_P_BoxType_eMax
} BMUXlib_File_MP4_P_BoxType;

/* Box nesting levels.  Top-level boxes do not nest inside other boxes (e.g. moov). */
typedef enum
{
   BMUXlib_File_MP4_P_BoxNesting_eTop = 0,
   BMUXlib_File_MP4_P_BoxNesting_eLevel1,
   BMUXlib_File_MP4_P_BoxNesting_eLevel2,
   BMUXlib_File_MP4_P_BoxNesting_eLevel3,
   BMUXlib_File_MP4_P_BoxNesting_eLevel4,
   BMUXlib_File_MP4_P_BoxNesting_eLevel5,
   BMUXlib_File_MP4_P_BoxNesting_eMax
} BMUXlib_File_MP4_P_BoxNesting;

/* box processing states ...
   each of the following states must stop processing wherever its possible to "block" due
   to lack of resources (buffer space, descriptor availability, etc).   This allows the same
   processing to be repeated when the resources become available  */
typedef enum
{
   BMUXlib_File_MP4_P_BoxState_eUnknown,
   /* generic box processing ... */
   BMUXlib_File_MP4_P_BoxState_eCreateBox,               /* create the fixed part of this box in memory */
   BMUXlib_File_MP4_P_BoxState_eAllocOutDescBox,         /* allocate an output descriptor for the fixed part of this box */
   BMUXlib_File_MP4_P_BoxState_eAllocOutDescBoxPart2,    /* allocate an output descriptor for the 2nd half of this box (if required) */
   BMUXlib_File_MP4_P_BoxState_eDoneBox,                 /* pseudo-state: current box has been completed - check for nested boxes */
   BMUXlib_File_MP4_P_BoxState_eFinishBox,               /* i.e. update the size field of this box */
   BMUXlib_File_MP4_P_BoxState_eAllocOutDescSize,        /* allocate an output descriptor for the updated size */
   /* mdat processing ... */
   BMUXlib_File_MP4_P_BoxState_eFinalizeMdat,            /* update the mdat size, then starting state for moov */
   BMUXlib_File_MP4_P_BoxState_eAllocMdatSizeDesc,       /* allocate a descriptor for updating mdat size */
   BMUXlib_File_MP4_P_BoxState_eAllocMdatSizeDescPart2,  /* allocate a descriptor for 2nd half of mdat size update (if required) */
   BMUXlib_File_MP4_P_BoxState_eCreateMoov,              /* pseudo-state: prep for creation of the moov */
   BMUXlib_File_MP4_P_BoxState_eRelocateMdat,            /* relocate the mdat from the temp temp to the main output, if required */
   /* metadata processing ... */
   BMUXlib_File_MP4_P_BoxState_eRetrieveMetadata,        /* read metadata back from storage for use in filling in the relevant box */
   BMUXlib_File_MP4_P_BoxState_eWaitForRead,             /* wait for the read transaction to complete */
   BMUXlib_File_MP4_P_BoxState_eAllocOutDescMetadata,    /* allocate a descriptor for updating a box with metadata content */
   BMUXlib_File_MP4_P_BoxState_eWaitForWrite,            /* wait for the write to box to complete before reading in more metadata */
   /* mdat relocation ... */
   /* FIXME: Optimization: reuse these states for read, wait, write, wait cycle */
   BMUXlib_File_MP4_P_BoxState_eRelocateMdatRead,
   BMUXlib_File_MP4_P_BoxState_eRelocateMdatWaitForRead,
   BMUXlib_File_MP4_P_BoxState_eRelocateMdatWrite,
   BMUXlib_File_MP4_P_BoxState_eRelocateMdatWaitForWrite,
   /* stco offset adjustments ... */
   BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsRead,
   BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsWaitForRead,
   BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsWrite,
   BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsWaitForWrite,
   BMUXlib_File_MP4_P_BoxState_eDone                     /* finished! do nothing - will enter this twice (after mdat creation, and at end of moov) */
} BMUXlib_File_MP4_P_BoxState;

typedef bool (*BMUXlib_File_MP4_P_CreateBoxFunction)(BMUXlib_File_MP4_Handle hMP4Mux);
typedef bool (*BMUXlib_File_MP4_P_BoxRequiredPredicate)(BMUXlib_File_MP4_Handle hMP4Mux);

/* the following is the information about the box to be created */
typedef struct
{
   BMUXlib_File_MP4_P_BoxNesting uiNestingLevel;         /* the nesting level of this container - top level boxes are level 0 */
   BMUXlib_File_MP4_P_BoxState eContentState;            /* the next state to create the contents of the box (if required)
                                                            will be "eUnknown" if no contents (i.e. box only contains other boxes) */
   BMUXlib_File_MP4_P_CreateBoxFunction pfCreateBox;     /* pointer to function to create this box (and all contained fixed size boxes) */
   BMUXlib_File_MP4_P_BoxRequiredPredicate pfIsRequired; /* pointer to predicate to determine if optional box is required
                                                            (NULL for mandatory boxes) */
} BMUXlib_File_MP4_P_BoxInfo;

/* box buffer allows for an extra 8 bytes (the "unwrap" space) beyond the end of the buffer to allow a single write
   to go beyond the end of the buffer, eliminating the need to check for buffer wrap per-byte (only required per-write)
   This allows for a single write up to 64 bits.  Usable space is uiBufSize - 1. */
typedef struct
{
   uint32_t uiBufSize;                                   /* size of buffer, not including the unwrap space */
   uint8_t *pBase;
   uint8_t *pEnd;                                        /* end of buffer - usable space extends 8 bytes beyond this */
   uint8_t *pTempWrite;                                  /* allows temporary update of the buffer incase any box writes fail */
   uint8_t *pWrite;
   uint8_t *pRead;
   /* information about the current box being constructed ... */
   uint32_t uiPart1Size;                                 /* current count of bytes in part1 of the box being written */
   uint32_t uiPart2Size;                                 /* current count of bytes in part2 - only valid if box wraps buffer boundary */
   uint8_t *pBoxStart;                                   /* pointer to start of box (part 2 is always pBase if Part2count != 0) */
   uint32_t *pSize;                                      /* points to either Part1Size or Part2Size, depending on wrap condition */

#if BDBG_DEBUG_BUILD
   uint32_t uiMaxUsage;
   uint32_t uiMaxInstUsage;                              /* maximum instantenous usage - maximum amount written at one time */
   uint32_t uiTotalBytesRequested;
#endif
} BMUXlib_File_MP4_P_BoxBuffer;

/* relocation buffer used as temporary storage during the relocation of the mdat and the update of offsets */
typedef struct
{
   uint32_t uiSize;
   uint8_t *pBase;
   uint32_t uiBytesUsed;
} BMUXlib_File_MP4_P_RelocationBuffer;

/* stack used to store current box to keep track of box nesting
   (boxes are pushed onto this stack when the box contains a nested box
   - allows update of the containing box's size when the box is complete) */
typedef struct
{
   uint32_t uiTopStack;
   BMUXlib_File_MP4_P_BoxType aData[BMUXlib_File_MP4_P_BoxNesting_eMax];
} BMUXlib_File_MP4_P_BoxStack;

/* stack used to store offsets to start of nested boxes that require size fixups
   stack entry is an offset in the output to box start (i.e. offset of the size)
   (offsets are pushed onto this stack whenever a new box is started, so the
   stack contains the starting offset of the box, to allow size calculation) */
typedef struct
{
   uint32_t uiTopStack;
   uint64_t aData[BMUXlib_File_MP4_P_BoxNesting_eMax];
} BMUXlib_File_MP4_P_OffsetStack;

/****************************
*    P R O T O T Y P E S    *
****************************/

BERR_Code   BMUXlib_File_MP4_P_ProcessBoxes(BMUXlib_File_MP4_Handle hMP4Mux);
void        BMUXlib_File_MP4_P_FreeBoxBufferData(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint8_t *pData, uint32_t uiSize);
#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_FILE_MP4_BOXES_H__ */

/*****************************************************************************
* EOF
******************************************************************************/
