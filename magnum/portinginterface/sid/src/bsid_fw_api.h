/******************************************************************************
* (c) 2004-2015 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
/*
    Common SID PI / ARC FW API
    Defines shared constants, structures between PI and FW.
    NOTE: Type differences are handled in the FW by including relevant
    typdefs prior to including this file

    *** This file is (intended to be) supplied as part of the SID FW release ***
*/

/* NOTE: We need to be careful of namespace conflict if items are defined here
   as well as in bsid.h - should FW-supplied values have a namespace with
   SID_ instead of BSID_ ? We could also define typedefs for structs to
   convert from FW names to PI names */


#ifndef BSID_FW_API_H__
#define BSID_FW_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************
/////////////////////// Defines, Typedef, Structs ////////////////////////////////
*********************************************************************************/

/* FIXME: Need to fix up these versions - FW version should be supplied by FW API, or in bsid_fw.c/h */
/* Versions:
    <Major>.<Minor>.<Patch>.<Build> (8 bits each)

    <Major> changes if API incompatibilities introduced
    <Minor> changes if features changed/added
    <Patch> changes if bug fixes made
    <Build> used to define special builds (such as engineering drop, etc)
*/

/* FIXME: do we need the API version - the API version is lock-step with the FW version
   (VCE has API version since same API might be used for multiple FW versions
   => it also uses the version from the command to verify against the version in the
   API header file to ensure they are correct (i.e. that PI is using same as FW
   => we can accomplish the same thing with FW version alone - the one in the API file
   should match that returned by the command) */
#define BSID_FW_API_VERSION     0x00010001
#define BSID_FW_VERSION         0x00010001

/* FIXME: This defined in sidapi.h */
/* FIXME: Seems like the number interrupts is the limiting factor on the number of channels */
/* This masks interrupts indicated in BCHP_SID_ARC_CPU_INTGEN_SET/CLR */
#define BSID_INTGEN_ENABLE_PINS                                     0x8000ffff /* mbx | sixteen channels enabled */

#define BSID_INT_CH_ENABLE                                          0x0000ffff

/**********************
  Error Codes
**********************/
/* FIXME: include the error codes used by the FW and passed back via responses */

/**********************
  Command Codes
***********************/

/**********************
  Limits
***********************/
/* FIXME: max command size
          max buffer sizes
          max channels
          max devices
          etc */


/* number of SID hw instance in the SOC. */
#define BSID_NUM_DEVICES 1

/* Number of channels supported by the FW */
/* FIXME: equiv. to BSID_MAX_CHANNEL defined in main.c */
/* Since ARC interrupts are used as per-channel interrupts, the number of channels is limited
   to the number of available interrupts (see Dispatch_isr() in bsid_msg.c)
   and BCHP_SID_ARC_CPU_INTGEN_CLR */
#define BSID_MAX_CHANNELS 16

/* Code sizes, total memory sizes, last code address, etc */

/* FIXME: This value needs to come from FW, or be determined by FW conversion
   What exactly does it signify?  Doesn't seem to match any defined values */
/* FIXME: Is this the same as ARC_CODE_SIZE? Seems to be (512K) */
#define BSID_FW_ARC_END_OF_CODE    (uint32_t)0x80000

/* FIXME: where are these generated from? FW creation?, linker? */
#define BSID_FW_TEXT_SEGMENT_OFFSET                               0x00000400 /* from FW linker */
#define BSID_FW_BOOT_SEGMENT_SIZE                                 0x00000400 /* from FW linker */
#define BSID_FW_DATA_SEGMENT_OFFSET                               0x00040000 /* from FW linker */
/* the following is fairly arbitrary - assumes total code size is less than this value
   - only used for clearing out the code memory prior to firmware authentication
   (existing code/boot length is 0x15b10 (88848 bytes); from bsid_fw.c) */
#define BSID_FW_BOOT_AND_TEXT_SEGMENT_EST_SIZE                      100*1024 /* 0x19000 */


/* FIXME: FW specific? */
/* NOTE: These values must be a multiple of 4 bytes to ensure the ImageHeader is
   a multiple of 32-bit words */
#define SID_GAMMA_CORRECTION_TABLE_ENTRIES     256
#define SID_MPEG2IFRAME_INTRA_QUANT_ENTRIES     64
#define SID_CLUT_MAX_ENTRIES                   256

/* NOTE: This assumes that the decode command is the largest command sent */
#define BSID_QUEUE_ENTRY_ELEM            sizeof(BSID_Cmd_Decode)/sizeof(uint32_t)

#if 0
/* FIXME: Not used */
#define BSID_P_CMD_MAXSIZE                                                      64 /* in 32-bit words */
#define BSID_P_RSP_MAXSIZE                                                      64
#define BSID_P_MSG_MAXSIZE                                                      64
#endif


/* FIXME: Same as SID_MSG_BASE in sidapi.h */
#define BSID_MSG_BASE                                                 (0x00700000)

/******************************************************************************/
/* FIXME: FW defines these values, but is currently using hard-coded "magic" numbers
   It should use these constants instead
   NOTE: how to deal with the PNG max width variation due to memory type?
   (see PNGReadIHead in sid_png.c) */
#define BSID_RLELEGACY_MAXWIDTH                                      720 /* pixel */
#define BSID_RLEHDDVD_MAXWIDTH                                      1920 /* pixel */
#define BSID_RLEBD_MAXWIDTH                                       8*1024 /* pixel */
#define BSID_JPEG_MAXWIDTH                                        8*1024 /* pixel */
#define BSID_PNGPALETTE_MAXWIDTH                                  8*1024 /* pixel */
#define BSID_PNGRGBA_MAXWIDTH                                     4*1024 /* pixel */
#define BSID_PNG16BPP_MAXWIDTH                                    2*1024 /* pixel */
#define BSID_GIF_MAXWIDTH                                         8*1024 /* pixel */

/* FIXME: This same macro as defined in main.c (used for consistency?) */
#define BSID_INCR_INDEX(index, maxValue) {  \
    if ((index + 1) >= maxValue) {          \
        index = 0;                          \
    }                                       \
    else {                                  \
        index ++;                           \
    }                                       \
}

/**********************
  Enumerations
**********************/

/* FIXME: Same as E_SID_MSG_CODE in sidapi.h */
typedef enum {
  BSID_MSG_HDR_REV            = (BSID_MSG_BASE + 0x0050), /* Not Used */
  BSID_MSG_HDR_TEST           = (BSID_MSG_BASE + 0x0051), /* Not Used */
  BSID_MSG_HDR_INIT           = (BSID_MSG_BASE + 0x0052), /* Blocking - waits for Response */
  BSID_MSG_HDR_DECODE         = (BSID_MSG_BASE + 0x0053), /* non-blocking */
  BSID_MSG_HDR_DECODESEGMENT  = (BSID_MSG_BASE + 0x0054), /* non-blocking */
  BSID_MSG_HDR_GETSTREAMINFO  = (BSID_MSG_BASE + 0x0055), /* non-blocking */
  BSID_MSG_HDR_OPENCHANNEL    = (BSID_MSG_BASE + 0x0057), /* blocking - waits for response */
  BSID_MSG_HDR_CLOSECHANNEL   = (BSID_MSG_BASE + 0x0059), /* blocking - waits for response */
  BSID_MSG_HDR_FLUSHCHANNEL   = (BSID_MSG_BASE + 0x0060), /* blocking - waits for response */
  BSID_MSG_HDR_SYNCCHANNEL    = (BSID_MSG_BASE + 0x0061), /* non-blocking */
  BSID_MSG_HDR_STARTDECODE    = (BSID_MSG_BASE + 0x0062), /* non-blocking (no response) */
  BSID_MSG_HDR_STOPDECODE     = (BSID_MSG_BASE + 0x0063)  /* Not Used */
} BSID_MSG_HDR;

/* FIXME: This is from err_codes.h
   Since these are passed back to the app via the event callback they
   need to be available for debugging purposes */
typedef enum  {
  ERR_CODE_SUCCESS = 0,
  /* NOTE: 0x00xx -> 0x1Fxx reserved for PI Errors */

  /* NOTE: any changes to this must result in corresponding change to
     error code checking in bsid_dbg.c, and additional errors added to bsid_err.h */
  ERR_CODE_GIF_BAD_FORMAT     = 0x2000,
  ERR_CODE_GIF_NO_COLOR_MAP   = 0x2001,
  ERR_CODE_GIF_NO_IMG_START   = 0x2003,
  ERR_CODE_GIF_BAD_ID         = 0x2004,
  ERR_CODE_GIF_ILLEGAL_SIZE   = 0x2005,
  ERR_CODE_GIF_BAD_EXTENSION  = 0x2006,
  ERR_CODE_GIF_EARLY_FILE_END = 0x2007,

  ERR_CODE_PNG_BAD_HEADER       = 0x3000,
  ERR_CODE_PNG_BAD_COMP_METHOD  = 0x3001,
  ERR_CODE_PNG_BAD_HUFFMAN      = 0x3002,
  ERR_CODE_PNG_BAD_BLK_TYPE_0   = 0x3003,
  ERR_CODE_PNG_BAD_INTERLACE    = 0x3004,
  ERR_CODE_PNG_ILLEGAL_SIZE     = 0x3005,
  ERR_CODE_PNG_BAD_BPP          = 0x3006,
  ERR_CODE_PNG_BAD_CUST_HUFFMAN = 0x3007,
  ERR_CODE_PNG_TOO_WIDE         = 0x3008,
  ERR_CODE_PNG_BAD_CRC          = 0x3009,
  ERR_CODE_PNG_BAD_TRANSPARENCY = 0x300a,
  ERR_CODE_PNG_BAD_PALETTE      = 0x300b,
  ERR_CODE_PNG_BAD_GAMA_CHUNK_SIZE = 0x300c,

  ERR_CODE_JPEG_BAD_HEADER           = 0x4000,
  ERR_CODE_JPEG_BAD_MARKER           = 0x4001,
  ERR_CODE_JPEG_NO_SOF_HUFF          = 0x4002,
  ERR_CODE_JPEG_UNKNOWN_FMT          = 0x4003,
  ERR_CODE_JPEG_BAD_NUM_COMPS        = 0x4004,
  ERR_CODE_JPEG_TOO_MANY_DEQUANT     = 0x4005,
  ERR_CODE_JPEG_UNKNOWN_HUFF_TC      = 0x4006,
  ERR_CODE_JPEG_UNKNOWN_HUFF_TH      = 0x4007,
  ERR_CODE_JPEG_BAD_HUFF_TABLE       = 0x4008,
  ERR_CODE_JPEG_BAD_QUANT            = 0x4009,
  ERR_CODE_JPEG_UNSUPP_TYPE          = 0x400a,
  ERR_CODE_JPEG_BAD_FRM_HEAD         = 0x400b,
  ERR_CODE_JPEG_BAD_RST_MARKER       = 0x400c,
  ERR_CODE_JPEG_BAD_SIZE             = 0x400d,
  ERR_CODE_JPEG_BAD_MARKER_SEGMENT   = 0x400e,
  ERR_CODE_JPEG_PROG_BAD_DC          = 0x400f,
  ERR_CODE_JPEG_PROG_BAD_AL_AH       = 0x4010,
  ERR_CODE_JPEG_PROG_BAD_HUFF_LOOKUP = 0x4011,
  ERR_CODE_JPEG_B_STREAM_OVER_READ   = 0x4012,

  ERR_CODE_RLE_BAD_FILE         = 0x5000,
  ERR_CODE_RLE_NO_COMMAND_7     = 0x5001,
  ERR_CODE_RLE_UNKNOWN_COMMAND  = 0x5002,
  ERR_CODE_RLE_BAD_SUBFMT       = 0x5003,
  ERR_CODE_RLE_BAD_HEADER       = 0x5004,
  ERR_CODE_RLEB_BAD_PALETTE     = 0x5005,
  ERR_CODE_RLEB_BAD_SEG_TYPE    = 0x5006,
  ERR_CODE_RLE_DCSQT_TOO_BIG    = 0x5007,
  ERR_CODE_RLE_BAD_SIZE         = 0x5008,
  ERR_CODE_RLE_BAD_DCSQT        = 0x5009,
  ERR_CODE_RLE_ILLEGAL_SIZE     = 0x500a,

  ERR_CODE_MIFRM_BAD_HEADER     = 0x5800,
  ERR_CODE_MIFRM_BAD_PICT_HEAD  = 0x5801,
  ERR_CODE_MIFRM_BAD_SLICE      = 0x5802,
  ERR_CODE_MIFRM_BAD_MBLOCK_1   = 0x5803,
  ERR_CODE_MIFRM_BAD_MBLOCK_2   = 0x5804,
  ERR_CODE_MIFRM_BAD_MBLOCK_3   = 0x5805,
  ERR_CODE_MIFRM_BAD_MBLOCK_4   = 0x5806,
  ERR_CODE_MIFRM_PICT_TOO_BIG   = 0x5807,
  ERR_CODE_MIFRM_BAD_MBTYPE     = 0x5808,
  ERR_CODE_MIFRM_FE_ERROR_FOUND = 0x5809,
  ERR_CODE_MIFRM_NO_COLOR_TAB   = 0x580a,

  ERR_CODE_BE_ERROR_FOUND       = 0x6000,

  ERR_CODE_SETUP_ERR           = 0x7000,
  ERR_CODE_UNSUPPORTED_FEATURE = 0x7001,
  ERR_CODE_ILLEGAL_STRIDE      = 0x7002,
  ERR_CODE_NO_STATE_BUFFER     = 0x7003,
  ERR_CODE_BAD_PARAMS          = 0x7004,
  ERR_CODE_IMG_TOO_BIG         = 0x7005,

  ERR_CODE_BAD_PIX_CNT = 0xe000,

  ERR_CODE_UNKNOWN_FMT = 0xf000,
  ERR_CODE_CMD_TOO_BIG = 0xf00f,
  ERR_CODE_WATCHDOG = 0xffff
  /* NOTE: any changes to this must result in corresponding change to
     error code checking in bsid_dbg.c, and additional errors added to bsid_err.h */
} ErrCode_t;


/* FIXME: Equivalent to outformat_t in image.h */
typedef enum {
  SID_OUT_YUVA       = 0,
  SID_OUT_YUYV       = 1,
  SID_OUT_GREY       = 2,
  SID_OUT_PALETTE    = 3,
  SID_OUT_RGBA       = 4,
  SID_OUT_GREY_ALPHA = 5,
  SID_OUT_LAST
} BSID_ArcOutFormat;

/* FIXME: These enums (from jpeg.h) are used in ImageHeader to be passed
   directly to the PI in eJpegSubType, so they need to be shared for verification
   that the enums on the FW and PI sides match */
typedef enum {
  JPEG_TYPE_GREY  = 0,
  JPEG_TYPE_444   = 1,
  JPEG_TYPE_422   = 2,
  JPEG_TYPE_420   = 3,

  /*  These are non-standard JPEG types, supported by post-processing */
  JPEG_TYPE_422r  = 4,
  JPEG_TYPE_411   = 5
} JpegImgType_t;

/* FIXME: Equivalent to E_SID_PICTURE_FORMAT in sidapi.h */
/* FW Expects file type to be lower 4 bits
   upper 4 bits are sub-type (only valid for RLE
   - for other file types, subtype determined from
   file header) */
typedef enum BSID_ArcImageFormat {
    SID_RLE_SVCD   = 0 | (3 << 4),
    SID_RLE_DVD    = 0 | (2 << 4),
    SID_RLE_BLURAY = 0 | (1 << 4),
    SID_RLE_HDDVD  = 0 | (0 << 4),
    SID_PNG        = 1,
    SID_GIF        = 2,
    SID_JPEG       = 3,
    /* SID_RLE_NO_HEADER = 4; (not supported) */
    SID_MPEG_IFRAME = 5
} BSID_ArcImageFormat;


/* FIXME: These are the same as BSID_ChannelState in main.c */
typedef enum BSID_ChannelState {
    /* channel not been fully opened or just closed. */
    BSID_ChannelState_eClose   = 0x0,
    /* channel has been opened, still channels were flushed, playback can begin/resume. */
    BSID_ChannelState_eReady   = 0x1,
    /* channel available for playback */
    BSID_ChannelState_eDecode  = 0x2,
    /* decode was suspended. This were still channels go.  */
    BSID_ChannelState_eSuspend = 0x3,
    /* channel has been stopped. Can be restarted if needed */
    BSID_ChannelState_eStop = 0x4
} BSID_ChannelState;

/* FIXME: These differ from the equivalent values in the FW (main.c) -
    BSID_OutputBufferState_eIdle    = 0x0,  no decode, no display
    BSID_OutputBufferState_eBooked  = 0x1,  booked for decoding
    BSID_OutputBufferState_eDecoded = 0x2,  decoded, no display
    BSID_OutputBufferState_eDisplay = 0x3   no decode, display
  - seems that although they are defined in the FW they are not used by the FW
  (i.e. output state is never written by FW) */
typedef enum BSID_OutputBufferState {
    BSID_OutputBufferState_eIdle    = 0x0,   /* no decode and no display */
    BSID_OutputBufferState_eDecode  = 0x1,   /* decoding */
    BSID_OutputBufferState_eDisplay = 0x2    /* display with xdm */
} BSID_OutputBufferState;

/* FIXME: what is this used for? */
/* FIXME: This is passed to the FW in InitCmd, but FW is using
   hard-coded "magic" numbers - it should use this enum */
/* FIXME: Can we obsolete this? What other kind of memory is there? */
typedef enum BSID_MemoryMode {
    BSID_MemoryMode_eNotUnifiedMemory = 0x0,
    BSID_MemoryMode_eUnifiedMemory    = 0x1,
    BSID_MemoryMode_eMemoryModeLast
} BSID_MemoryMode;

/*********************
*********************/
/* FIXME: definitions of release queues, picture queues, etc all need to be common - they are referenced in shared memory
   NOTE: there are dangerous definitions in these structs - enums are not portable - they cannot be guaranteed to have the same
   size in memory? FW assumes enum is 32-bits but it may not be so on the PI side (depending on compiler). Yet these
   structures are memcpy'd! */

/* FIXME: from sidapi.h */
typedef struct SID_ImageHeader {
    uint32_t    ePixelFormat; /* see BSID_ArcOutFormat */
    uint32_t    ui32_GrayScale;
    uint32_t    ui32_BitPerPixel;
    uint32_t    ui32_ImgWidth;
    uint32_t    ui32_ImgHeight;
    uint32_t    ui32_SurPitch;
    uint32_t    ui32_SurWidth;
    uint32_t    ui32_SurHeight;
    uint32_t    ui32_MultiScan;
    uint32_t    ui32_NumClutEntries;
    uint32_t    ae_Clut[SID_CLUT_MAX_ENTRIES];
    uint32_t    eClutPixelFormat; /* see BSID_ArcOutFormat */
    uint32_t    eJpegSubType;   /* see JpegImgType_t */
    uint32_t    ui32_isIntraQuantTablePresent;
    uint8_t     ui8_intra_quant_table[SID_MPEG2IFRAME_INTRA_QUANT_ENTRIES];
    uint32_t    ui32_pic_struct;
    uint32_t    ui32_gamma;
    uint32_t    ui32_HasTransparentData;
    uint32_t    ui32_TransparentColorIndex;
    uint32_t    ui32_TransparentColorRGB;
} SID_ImageHeader;


/********************************************************************************/
/* FIXME: These command/response structures are the same as their equivalent
   versions in sidapi.h (e.g. ST_BSID_INIT_CMD) */

/********************************************************************************/
typedef struct BSID_Cmd_Hdr {
    uint32_t header;
    uint32_t sequence;
    uint32_t needAck;
} BSID_Cmd_Hdr;


/********************************************************************************/
typedef struct BSID_Rsp_Hdr {
    uint32_t header;
    uint32_t sequence;
    uint32_t errCode;
} BSID_Rsp_Hdr;

/********************************************************************************/
/*typedef struct BSID_Msg_Hdr {
    uint32_t header;
    uint32_t sequence;
    uint32_t errCode;
} BSID_Msg_Hdr;*/


/********************************************************************************/
/* FIXME: generic command structure - not used
typedef struct BSID_Cmd {
    BSID_Cmd_Hdr cmdHdr;
    uint32_t     cmdBody[ BSID_P_CMD_MAXSIZE - (sizeof(BSID_Cmd_Hdr)/sizeof(uint32_t)) ];
} BSID_Cmd;*/


/********************************************************************************/
/* FIXME: generic response structure - not used
typedef struct BSID_Rsp {
    BSID_Rsp_Hdr rspHdr;
    uint32_t     rspBody[ BSID_P_RSP_MAXSIZE - (sizeof(BSID_Rsp_Hdr)/sizeof(uint32_t)) ];
} BSID_Rsp;*/


/********************************************************************************/
/* FIXME: This "message" does not seem to be used */
/*
typedef struct BSID_Msg {
    BSID_Msg_Hdr msgHdr;
    uint32_t     msgBody[ BSID_P_MSG_MAXSIZE - (sizeof(BSID_Msg_Hdr)/sizeof(uint32_t)) ];
} BSID_Msg;*/

/* Message Command: INIT */
typedef struct BSID_Cmd_Init {
   BSID_Cmd_Hdr cmdHdr;
   uint32_t     frequency;
   uint32_t     baudRate;
   uint32_t     mbxCmdBufAddr;
   uint32_t     mbxRspBufAddr;
   uint32_t     mbxCmdRspBufSize;
   uint32_t     memMode;
   uint32_t     memBase;
   uint32_t     memSize;
   uint32_t     msBufBase;
   uint32_t     msBufSize;
   uint32_t     dmaInfoBufAddr;
   uint32_t     dmaInfoBufSize;
   uint32_t     endianessSwap;
   uint32_t     jpegFiltMode;
   uint32_t     alphaValue;
   uint32_t     dbgMsgOn;
} BSID_Cmd_Init;

/* Message Response: INIT */
typedef struct BSID_Rsp_Init {
   BSID_Rsp_Hdr rspHdr;
   /* FIXME: we should add FW and HW version info here */
} BSID_Rsp_Init;


/********************************************************************************/
/* Message Command: DECODE */
typedef struct BSID_Cmd_Decode {
   BSID_Cmd_Hdr cmdHdr;
   uint32_t     imgFormat;
   uint32_t     imgWidth;
   uint32_t     imgHeight;
   uint32_t     imgPitch;
   uint32_t     imgBpp;
   uint32_t     sidMemImgBufAddr;
   uint32_t     sidMemImgBufSize;
   uint32_t     sidMemInpBufAddr0;
   uint32_t     sidMemInpBufSize0;
   uint32_t     sidMemInpBufAddr1;
   uint32_t     sidMemInpBufSize1;
   uint32_t     segMaxHeight;
   uint32_t     firstSegment;
   uint32_t     dbgRleParseHeader;
   uint32_t     dbgMsgOn;
   uint32_t 	 isIntraQuantTablePresent;
   uint32_t     lastDMAchunk;
   uint32_t     msBufBase;
   uint32_t     msBufSize;
   uint8_t   	 intra_quant_table[SID_MPEG2IFRAME_INTRA_QUANT_ENTRIES];
   uint8_t      gamma_correction_table[SID_GAMMA_CORRECTION_TABLE_ENTRIES];
   uint32_t     bypass;
   uint32_t     doCheckSum;
} BSID_Cmd_Decode;

/* Message Response: DECODE */
typedef struct BSID_Rsp_Decode {
   BSID_Rsp_Hdr rspHdr;
} BSID_Rsp_Decode;


/********************************************************************************/
/* Message Command: GETSTREAMINFO*/
typedef struct BSID_Cmd_GetStreamInfo {
   BSID_Cmd_Hdr cmdHdr;
   uint32_t     imgFormat;
   uint32_t     sidMemInpBufAddr0;
   uint32_t     sidMemInpBufSize0;
   uint32_t     sidMemInpBufAddr1;
   uint32_t     sidMemInpBufSize1;
   uint32_t     sidMemStreamInfoBufPhysAddr;
   uint32_t     sidMemStreamInfoBufVirtAddr; /* FIXME: These are virtual addresses, so we have an issue when converting on a 64-bit system */
   uint32_t     sidMemStreamInfoBufSize;
   uint32_t     streamInfoBufVirtAddr;
   uint32_t     bypass;
   uint32_t     lastDMAchunk;
   uint32_t     dbgMsgOn;
} BSID_Cmd_GetStreamInfo;


/* Message Response: GETSTREAMINFO */
typedef struct BSID_Rsp_GetStreamInfo {
   BSID_Rsp_Hdr rspHdr;
} BSID_Rsp_GetStreamInfo;

/********************************************************************************/
/* Message Command: DECODESEGMENT */
typedef struct BSID_Cmd_Decode BSID_Cmd_DecodeSegment;


/* Message Response: DECODESEGMENT */
typedef struct BSID_Rsp_Decode BSID_Rsp_DecodeSegment;

/********************************************************************************/
/* Message Command: OPENCHANNEL */
typedef struct BSID_Cmd_OpenChannel {
   BSID_Cmd_Hdr cmdHdr;
   uint32_t     channelID;
   uint32_t     channelNumber;
   uint32_t     priority;
   uint32_t     reqQueueAddr;
   uint32_t     relQueueAddr;
   uint32_t     queueDepth;
   uint32_t     dbgMsgOn;
} BSID_Cmd_OpenChannel;

/* Message Response: OPENCHANNEL */
typedef struct BSID_Rsp_OpenChannel {
   BSID_Rsp_Hdr rspHdr;
} BSID_Rsp_OpenChannel;

/********************************************************************************/
/* Message Command: CLOSECHANNEL */
typedef struct BSID_Cmd_CloseChannel {
   BSID_Cmd_Hdr cmdHdr;
   uint32_t     channelID;
   uint32_t     channelNumber;
   uint32_t     dbgMsgOn;
} BSID_Cmd_CloseChannel;

/* Message Response: CLOSECHANNEL */
typedef struct BSID_Rsp_CloseChannel {
   BSID_Rsp_Hdr rspHdr;
} BSID_Rsp_CloseChannel;

/********************************************************************************/
/* Message Command: FLUSHCHANNEL */
typedef struct BSID_Cmd_FlushChannel {
   BSID_Cmd_Hdr cmdHdr;
   uint32_t     channelID;
   uint32_t     channelNumber;
   uint32_t     dbgMsgOn;
} BSID_Cmd_FlushChannel;

/* Message Response: FLUSHCHANNEL */
typedef struct BSID_Rsp_FlushChannel {
   BSID_Rsp_Hdr rspHdr;
} BSID_Rsp_FlushChannel;

/********************************************************************************/
/* Message Command: STARTDECODE */
typedef struct BSID_Cmd_StartDecode {
   BSID_Cmd_Hdr cmdHdr;
    uint32_t    channelID;
    uint32_t    channelNumber;
    uint32_t    imgFormat;
    uint32_t    raveReportBufAddr;
    uint32_t    playbackReadQueueAddr;
    uint32_t    playbackWriteQueueAddr;
    uint32_t    queueDepth;
    uint32_t    sidMemStreamInfoBufPhysAddr;
    uint32_t    sidMemStreamInfoBufVirtAddr;
    uint32_t    sidMemStreamInfoBufSize;
    uint32_t    numPicToDecode;
    uint32_t    endianessInput;
    uint32_t    endianessOutput;
    uint32_t    bypass;
    uint32_t    dbgMsgOn;
} BSID_Cmd_StartDecode;

/* Message Response: STARTDECODE */
typedef struct BSID_Rsp_StartDecode {
   BSID_Rsp_Hdr rspHdr;
} BSID_Rsp_StartDecode;

/********************************************************************************/
/* Message Command: STOPDECODE */
typedef struct BSID_Cmd_StopDecode {
   BSID_Cmd_Hdr cmdHdr;
   uint32_t     channelID;
   uint32_t     channelNumber;
   uint32_t     dbgMsgOn;
} BSID_Cmd_StopDecode;

/* Message Response: STOPDECODE */
typedef struct BSID_Rsp_StopDecode {
   BSID_Rsp_Hdr rspHdr;
} BSID_Rsp_StopDecode;

/********************************************************************************/
/* Message Command: SYNCCHANNEL */
typedef struct BSID_Cmd_SyncChannel {
   BSID_Cmd_Hdr cmdHdr;
   uint32_t     channelID;
   uint32_t     channelNumber;
   uint32_t     bypass;
   uint32_t     dbgMsgOn;
} BSID_Cmd_SyncChannel;

/* Message Response: SYNCCHANNEL */
typedef struct BSID_Rsp_SyncChannel {
   BSID_Rsp_Hdr rspHdr;
} BSID_Rsp_SyncChannel;


/* These same as defintions from main.c */
typedef struct BSID_ReqQueueEntry {
   BSID_Cmd_Hdr cmdHeader;
   uint32_t     entry[BSID_QUEUE_ENTRY_ELEM];
} BSID_ReqQueueEntry;

typedef struct BSID_RelQueueEntry {
   BSID_Rsp_Hdr rspHeader;
   uint32_t     sidMemStreamInfoBufVirtAddr;
   uint32_t     streamInfoBufVirtAddr;
   uint32_t     checkSumValue;
   uint32_t     operationCompleted;
} BSID_RelQueueEntry;

/* FIXME: this is same as BSID_QueueHeader in main.c */
typedef struct BSID_CommandQueueHeader {
    uint32_t ui32_ReadIndex;
    uint32_t ui32_WriteIndex;
    uint32_t ui32_WaitForData;
} BSID_CommandQueueHeader;

/* FIXME: These are common with the types defined in main.c */
typedef struct BSID_PlaybackWriteQueueEntry {
    uint32_t ui32_CdbAddress;
    uint32_t ui32_CdbSize;
    uint32_t ui32_Pts;
    uint32_t ui32_OutputBufferAddress;
    uint32_t ui32_OutputBufferMaxSize;
} BSID_PlaybackWriteQueueEntry;

typedef struct BSID_PlaybackWriteQueueHeader {
    uint32_t ui32_NewPictWriteIndex;
    uint32_t ui32_ChannelState;
} BSID_PlaybackWriteQueueHeader;

typedef struct BSID_PlaybackReadQueueEntry {
    uint32_t ui32_PictureIdx;
    uint32_t ui32_Width;
    uint32_t ui32_Height;
    uint32_t ui32_Pitch;
    uint32_t ui32_PixelFormat;
    uint32_t ui32_BitPerPixel;
    uint32_t ui32_TrueWidth;
    uint32_t ui32_TrueHeight;
    uint32_t ui32_DecodeErrors;
    uint32_t ui32_setup_time;
    uint32_t ui32_decode_time;
    uint32_t ui32_WrapAroundOccurred;
    uint32_t ui32_JpegType;
    uint32_t ui32_LastCdbRead;
} BSID_PlaybackReadQueueEntry;

typedef struct BSID_PlaybackReadQueueHeader {
    uint32_t ui32_DecodedReadIndex;
    uint32_t ui32_ChannelState;
} BSID_PlaybackReadQueueHeader;

/* NOTE: This API cannot share the actual definitions of
   the channel queue or playback queue, etc since the
   definitions of linear buffer are different.
   Only the queue entries are actually passed between
   host and Arc (via addresses in the commands) */

#ifdef __cplusplus
}
#endif

#endif /* BSID_FW_API_H__ */

/***********************************
         End of File
************************************/
