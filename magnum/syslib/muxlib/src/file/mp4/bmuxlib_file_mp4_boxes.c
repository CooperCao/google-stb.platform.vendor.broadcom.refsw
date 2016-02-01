/***************************************************************************
 *     Copyright (c) 2010-2013, Broadcom Corporation
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
 * Box Creation functions for File-based MP4 software mux
 *
 * NOTE: MP4 is a big-endian format!
 *
 * Operational details:
 * ====================
 *
 *
 * Typical file structure:
 * =======================
 * ftyp (m)                [fixed - can create brands based on content]
 * pdin (?)                [required only for assisting streamers]
 * moov (m)                [container only]
 *   mvhd (m)              [createtime, modtime, timescale, duration, next_track_ID]
 *   trak (t)              [container only]
 *      tkhd               [createtime, modtime, trackID, duration, width/height]
 *      tref (?)           [only required if this track references another - e.g. for Parameter track (media track references the param track)]
 *      edts               [container only]
 *         elst            [specifies initial offset between tracks]
 *      mdia               [container only]
 *         mdhd            [createtime, modtime, timescale (fixed at 90kHz for video, sample rate for audio), duration, language (currently set to "unk")]
 *         hdlr            [handler type (based on track type), no other info required]
 *         minf            [container only]
 *            vmhd/smhd/etc  [typically, these contain predefined information]
 *            dinf           [container only]
 *               dref        [URL - empty]
 *            stbl           [container only]
 *               stsd        [codec-specific information]
 *                  avc1     [AVC only]
 *                     avcc  [AVC only]
 *                  mp4a/v/s [MPEG 4 only]
 *                     esds  [MPEG 4 only - contains an ES Descriptor and its Decoder Specific Config]
 *               stts        [from stored metadata - DTS deltas]
 *               ctts (?)    [from stored metadata - DTS/CTS offset]
 *               stsc        [from analysis of sample offsets to create chunks]
 *               stsz        [from stored metadata - sample size]
 *               stco        [from stored metadata - mdat offsets]
 *               stss (?)    [from stored metadata - RAP indications]
 *               padb (?)    [only needed if encoder output needs byte alignment]
 *    udta (?)
 *       cprt (?)  [might be required by customers]
 * mdat (m)   [container only]
 *
 *  (m) = once per file
 *  (t) = once per track
 *  (?) = optional or may not be needed
 *
 * Box Creation Steps:
 * ===================
 * NOTE: Here a "box" is an unbounded box - i.e. their sizes are not fixed,
 * and not known when the box is created.
 * Fixed size boxes are incorporated into the processing of other
 * unbounded boxes.
 * (e.g. mvhd is created when the moov is created)
 * This is done to allow space allocation within the box buffer for
 * boxes of known size, and allow the following step-wise process for
 * boxes of unknown size (unbounded boxes)
 *
 *   - Create Box (push start offset - location of the size)
 *   - Alloc Out Desc (for the known-size part of the box)
 *   - Create Box contents (optional - some boxes only contain other boxes)
 *   if (box does not contains other boxes)
 *     - Calculate Updated Size(pop offset, and update location with actual size)
 *     - Allocate Out Desc (for the size)
 *   - next box
 *
 * Note that the processing can block at each step due to I/O or lack of
 * resources:
 *  - Create Box blocks due to lack of space in box buffer
 *  - Create Box Contents blocks due to either lack of storage or descriptors
 *  - Alloc Out Desc blocks due to lack of descriptors
 *  - Calculate Updated Size blocks due lack of room in the size buffer
 *
 * If blocking occurs, DoMux() must relinquish control, but processing
 * MUST return to the same point to allow the box to be completed when
 * resources become available.
 * Hence, the moov and other box creation is state-based processing
 * (see BMUXlib_File_MP4_P_ProcessBoxes).
 *
 ***************************************************************************
 * [Revision History:]
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h" /* also includes berr, bdbg, etc */
#include "bkni.h"

#include "bmuxlib_file_mp4_priv.h"
#include "bmuxlib_file_mp4_boxes.h"

BDBG_MODULE(BMUXLIB_FILE_MP4_BOXES);
BDBG_FILE_MODULE(BMUX_MP4_BOXBUFFER);  /* enables box buffer diagnostics */
BDBG_FILE_MODULE(BMUX_MP4_STACK);      /* enables box offset stack diagnostics */
BDBG_FILE_MODULE(BMUX_MP4_STATE);      /* enables state machine diagnostics */

/* Function to use to write a FourCC */
#define BoxBufferWriteFourCC BoxBufferWriteU32

/****************************
     Static Prototypes
****************************/
static void VerifyTrackMetadata(BMUXlib_File_MP4_P_TrackInfo *pTrack);

static void CalculateFinalizationSteps(BMUXlib_File_MP4_Handle hMP4Mux);

static BMUXlib_File_MP4_P_BoxState CheckForNestedBoxes(BMUXlib_File_MP4_Handle hMP4Mux);
static BMUXlib_File_MP4_P_BoxState CheckForMoreBoxes(BMUXlib_File_MP4_Handle hMP4Mux);

static bool IsCttsRequired(BMUXlib_File_MP4_Handle hMP4Mux);
static bool IsStssRequired(BMUXlib_File_MP4_Handle hMP4Mux);
static bool IsAlwaysRequired(BMUXlib_File_MP4_Handle hMP4Mux);

static bool CreateBoxFtyp(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxMdat(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxMoov(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxMvhd(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxHdlr(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxTrak(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxTkhd(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxEdts(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxElst(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxMdia(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxMdhd(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxMinf(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxVmhd(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxSmhd(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxHmhd(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxNmhd(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxDinf(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxDref(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxStbl(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxStsd(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxStts(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxCtts(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxStsc(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxStsz(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxStco(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxStss(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxAvcC(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxEsds(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxDataEntryUrl(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxVideoSampleEntry(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxAudioSampleEntry(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxMpeg4SampleEntry(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateBoxSampleEntry(BMUXlib_File_MP4_Handle hMP4Mux, uint32_t uiPLSize);

static bool CreateBox(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, BMUXlib_File_MP4_FourCC uiBoxType, uint32_t uiBoxPLSize);
static bool CreateLargeBox(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, BMUXlib_File_MP4_FourCC uiBoxType, uint32_t uiBoxPLSize);
static bool CreateFullBox(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, BMUXlib_File_MP4_FourCC uiBoxType, uint32_t uiBoxSize,
                  uint8_t uiVersion, uint32_t uiFlags);

static bool CreateMpeg4DescES(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateMpeg4DescDecConfig(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateMpeg4DescDecSpecInfo(BMUXlib_File_MP4_Handle hMP4Mux);
static bool CreateMpeg4DescSLConfig(BMUXlib_File_MP4_Handle hMP4Mux);

static bool CreateMpeg4Desc(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint8_t uiDescTag, uint32_t uiDescPLSize);

static uint32_t BoxBufferGetSpace(BMUXlib_File_MP4_P_BoxBuffer *pBuffer);
static void BoxBufferStartBox(BMUXlib_File_MP4_P_BoxBuffer *pBuffer);
static void BoxBufferWriteU8(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint8_t uiByte);
static void BoxBufferWriteU16(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint16_t uiData);
static void BoxBufferWriteU32(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint32_t uiData);
static void BoxBufferWriteU64(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint64_t uiData);
static void BoxBufferWriteDescSize(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint32_t uiSize);
static void BoxBufferWriteMatrix(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, BMUXlib_File_MP4_P_Matrix *pMatrix);
static void BoxBufferFill(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint8_t uiData, uint32_t uiCount);
static void *BoxBufferWrapCheck(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, void *pWrite);
static void BoxBufferFinishBox(BMUXlib_File_MP4_P_BoxBuffer *pBuffer);

static bool OffsetStackPush(BMUXlib_File_MP4_P_OffsetStack *pStack, uint64_t uiOffset);
static bool OffsetStackPop(BMUXlib_File_MP4_P_OffsetStack *pStack, uint64_t *puiOffset);
static bool OffsetStackPeek(BMUXlib_File_MP4_P_OffsetStack *pStack, uint64_t *puiOffset);
static bool OffsetStackModify(BMUXlib_File_MP4_P_OffsetStack *pStack, uint64_t uiOffset);

/**************************
    Static Definitions
**************************/
/* identity matrix =
      a  b  u   =   1.0  0.0  0.0
      c  d  v       0.0  1.0  0.0
      x  y  w       0.0  0.0  1.0

   Note: all values 16.16 fixed point EXCEPT u, v and w which are 2.30 fixed
*/
static const BMUXlib_File_MP4_P_Matrix IdentityMatrix =
/* Stored order: a, b, u, c, d, v, x, y, w */
{  0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };

/* array of information for each unbounded box to be created */
static const BMUXlib_File_MP4_P_BoxInfo BoxInfoTable[BMUXlib_File_MP4_P_BoxType_eMax] =
{
   /* nesting level, state for content creation, processing function, box required predicate */
   { BMUXlib_File_MP4_P_BoxNesting_eTop,    BMUXlib_File_MP4_P_BoxState_eUnknown,          NULL          , NULL              },  /* BMUXlib_File_MP4_P_BoxType_eUnknown */
   { BMUXlib_File_MP4_P_BoxNesting_eTop,    BMUXlib_File_MP4_P_BoxState_eUnknown,          &CreateBoxFtyp, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eFtyp */
   { BMUXlib_File_MP4_P_BoxNesting_eTop,    BMUXlib_File_MP4_P_BoxState_eUnknown,          &CreateBoxMdat, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eMdat */
   /* --- break here ---
      processing of input data happens between creation of mdat and creation of moov.
      Processing does not continue until all input data is done */
   { BMUXlib_File_MP4_P_BoxNesting_eTop,    BMUXlib_File_MP4_P_BoxState_eUnknown,          &CreateBoxMoov, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eMoov */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel1, BMUXlib_File_MP4_P_BoxState_eUnknown,          &CreateBoxTrak, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eTrak */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel2, BMUXlib_File_MP4_P_BoxState_eUnknown,          &CreateBoxMdia, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eMdia */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel3, BMUXlib_File_MP4_P_BoxState_eUnknown,          &CreateBoxMinf, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eMinf */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel4, BMUXlib_File_MP4_P_BoxState_eUnknown,          &CreateBoxStbl, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eStbl */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel5, BMUXlib_File_MP4_P_BoxState_eUnknown,          &CreateBoxStsd, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eStsd */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel5, BMUXlib_File_MP4_P_BoxState_eRetrieveMetadata, &CreateBoxStts, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eStts */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel5, BMUXlib_File_MP4_P_BoxState_eRetrieveMetadata, &CreateBoxCtts, &IsCttsRequired   },  /* BMUXlib_File_MP4_P_BoxType_eCtts */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel5, BMUXlib_File_MP4_P_BoxState_eUnknown,          &CreateBoxStsc, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eStsc */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel5, BMUXlib_File_MP4_P_BoxState_eRetrieveMetadata, &CreateBoxStsz, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eStsz */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel5, BMUXlib_File_MP4_P_BoxState_eRetrieveMetadata, &CreateBoxStco, &IsAlwaysRequired },  /* BMUXlib_File_MP4_P_BoxType_eStco */
   { BMUXlib_File_MP4_P_BoxNesting_eLevel5, BMUXlib_File_MP4_P_BoxState_eRetrieveMetadata, &CreateBoxStss, &IsStssRequired   }   /* BMUXlib_File_MP4_P_BoxType_eStss */
};

/* this array defines the sequence of unbounded boxes to create - the state machine can jump
   around in here as needed to repeat sequences (for example, to create each trak box), etc.
   Any fixed size boxes that occur within these boxes are typically written at the time the
   unbounded box is created */
static const BMUXlib_File_MP4_P_BoxType BoxSequenceTable[] =
{
   BMUXlib_File_MP4_P_BoxType_eFtyp,
   /* pdin would be placed here, if required */
   /* NOTE: mdat creation is done "upfront" to allow "moov at end"
      (mdat contents are written to the main output, followed by the moov)
      otherwise if moov is desired at start (progressive download compatible)
      then the mdat is written to another output, the moov is created in
      the main output file, and the mdat is relocated to follow to moov
      when complete) */
   BMUXlib_File_MP4_P_BoxType_eMdat,
   /* --- break here ---
      processing of input data happens between creation of mdat and creation of moov.
      Box Processing does not continue until all input data is done */
   BMUXlib_File_MP4_P_BoxType_eMoov,      /* moov/mvhd */
   BMUXlib_File_MP4_P_BoxType_eTrak,      /* trak/tkhd/edts/elst */
   /* tref would go here (if unbounded) or would be part of trak (after tkhd) if fixed size */
   BMUXlib_File_MP4_P_BoxType_eMdia,      /* mdia/mdhd/hdlr */
   BMUXlib_File_MP4_P_BoxType_eMinf,      /* minf/vmhd/smhd/hmhd/nmhd/dinf/dref */
   BMUXlib_File_MP4_P_BoxType_eStbl,      /* actually contains the following boxes ... */
   BMUXlib_File_MP4_P_BoxType_eStsd,
   BMUXlib_File_MP4_P_BoxType_eStts,
   BMUXlib_File_MP4_P_BoxType_eCtts,
   BMUXlib_File_MP4_P_BoxType_eStsc,
   BMUXlib_File_MP4_P_BoxType_eStsz,
   BMUXlib_File_MP4_P_BoxType_eStco,
   BMUXlib_File_MP4_P_BoxType_eStss,
   /* --- break here ---
      this indicates the end of the trak, processing will loop for other traks, as required */
   /* --- if needed other boxes, such as mvex, moof, etc can be added here --- */
   BMUXlib_File_MP4_P_BoxType_eUnknown
};

static const BMUXlib_File_MP4_P_MetadataType BoxToMetadataTable[BMUXlib_File_MP4_P_BoxType_eMax] =
{
   /* the following entries should never occur (these boxes do not use metadata) ... */
   BMUXlib_File_MP4_P_MetadataType_eMax,     /* BMUXlib_File_MP4_P_BoxType_eUnknown */
   BMUXlib_File_MP4_P_MetadataType_eMax,     /* BMUXlib_File_MP4_P_BoxType_eFtyp */
   BMUXlib_File_MP4_P_MetadataType_eMax,     /* BMUXlib_File_MP4_P_BoxType_eMdat */
   BMUXlib_File_MP4_P_MetadataType_eMax,     /* BMUXlib_File_MP4_P_BoxType_eMoov */
   BMUXlib_File_MP4_P_MetadataType_eMax,     /* BMUXlib_File_MP4_P_BoxType_eTrak */
   BMUXlib_File_MP4_P_MetadataType_eMax,     /* BMUXlib_File_MP4_P_BoxType_eMdia */
   BMUXlib_File_MP4_P_MetadataType_eMax,     /* BMUXlib_File_MP4_P_BoxType_eMinf */
   BMUXlib_File_MP4_P_MetadataType_eMax,     /* BMUXlib_File_MP4_P_BoxType_eStbl */
   BMUXlib_File_MP4_P_MetadataType_eMax,     /* BMUXlib_File_MP4_P_BoxType_eStsd */
   /* ---------------------------------*/
   BMUXlib_File_MP4_P_MetadataType_eStts,    /* BMUXlib_File_MP4_P_BoxType_eStts */
   BMUXlib_File_MP4_P_MetadataType_eCtts,    /* BMUXlib_File_MP4_P_BoxType_eCtts */
   BMUXlib_File_MP4_P_MetadataType_eMax,     /* BMUXlib_File_MP4_P_BoxType_eStsc - does not use metadata */
   BMUXlib_File_MP4_P_MetadataType_eStsz,    /* BMUXlib_File_MP4_P_BoxType_eStsz */
   BMUXlib_File_MP4_P_MetadataType_eStco,    /* BMUXlib_File_MP4_P_BoxType_eStco */
   BMUXlib_File_MP4_P_MetadataType_eStss     /* BMUXlib_File_MP4_P_BoxType_eStss */
};

/*
   Conversion table for converting the track type into the handler (fourcc) to
   be used in creation of the hdlr box
*/
static const BMUXlib_File_MP4_FourCC TrackToHandlerTable[] =
{
   0,                                        /* BMUXlib_File_MP4_P_TrackType_eUnknown */
   BMUXLIB_FILE_MP4_P_HANDLER_VIDE,          /* BMUXlib_File_MP4_P_TrackType_eVideo */
   BMUXLIB_FILE_MP4_P_HANDLER_SOUN,          /* BMUXlib_File_MP4_P_TrackType_eAudio */
   BMUXLIB_FILE_MP4_P_HANDLER_HINT,          /* BMUXlib_File_MP4_P_TrackType_eHint */
   BMUXLIB_FILE_MP4_P_HANDLER_ODSM,          /* BMUXlib_File_MP4_P_TrackType_eODSM */
   BMUXLIB_FILE_MP4_P_HANDLER_SDSM           /* BMUXlib_File_MP4_P_TrackType_eSDSM */
};

/*
   Conversion table for converting the track type into the stream type
   used in MPEG 4 Decoder Config descriptors
*/
static const uint8_t TrackToStreamTypeTable[] =
{
   0,                                        /* BMUXlib_File_MP4_P_TrackType_eUnknown */
   BMUXLIB_FILE_MP4_P_STR_TYPE_VIS,          /* BMUXlib_File_MP4_P_TrackType_eVideo */
   BMUXLIB_FILE_MP4_P_STR_TYPE_AUD,          /* BMUXlib_File_MP4_P_TrackType_eAudio */
   0,                                        /* BMUXlib_File_MP4_P_TrackType_eHint (invalid - hint tracks do not use a DecConfigDesc) */
   BMUXLIB_FILE_MP4_P_STR_TYPE_ODS,          /* BMUXlib_File_MP4_P_TrackType_eODSM */
   BMUXLIB_FILE_MP4_P_STR_TYPE_SDS           /* BMUXlib_File_MP4_P_TrackType_eSDSM */
};

/*
   Conversion table for converting the coding type to the format (fourcc) to be used
   to create the box that extends either the VideoSampleEntry, AudioSampleEntry, or
   SampleEntry
*/
static const BMUXlib_File_MP4_FourCC CodingToFormatTable[] =
{
   0,                                        /* BMUXlib_File_MP4_P_CodingType_eUnknown */
   BMUXLIB_FILE_MP4_P_BOX_TYPE_AVC1,         /* BMUXlib_File_MP4_P_CodingType_eAVC */
   BMUXLIB_FILE_MP4_P_BOX_TYPE_MP4A,         /* BMUXlib_File_MP4_P_CodingType_eMpeg4Audio */
   BMUXLIB_FILE_MP4_P_BOX_TYPE_MP4V,         /* BMUXlib_File_MP4_P_CodingType_eMpeg4Video */
   BMUXLIB_FILE_MP4_P_BOX_TYPE_MP4S          /* BMUXlib_File_MP4_P_CodingType_eMpeg4Systems */
};

static const uint8_t CodingToObjTypeTable[] =
{
   0,                                        /* BMUXlib_File_MP4_P_CodingType_eUnknown */
   BMUXLIB_FILE_MP4_P_OBJ_TYPE_AVC,          /* BMUXlib_File_MP4_P_CodingType_eAVC */
   BMUXLIB_FILE_MP4_P_OBJ_TYPE_MPEG4_AUD,    /* BMUXlib_File_MP4_P_CodingType_eMpeg4Audio */
   BMUXLIB_FILE_MP4_P_OBJ_TYPE_MPEG4_VID,    /* BMUXlib_File_MP4_P_CodingType_eMpeg4Video */
   BMUXLIB_FILE_MP4_P_OBJ_TYPE_MPEG4_SYS     /* BMUXlib_File_MP4_P_CodingType_eMpeg4Systems */
};

#if BDBG_DEBUG_BUILD
static const char * const DebugBoxTypeTable[] =
{
   "UNKNOWN",
   "ftyp",
   "mdat",
   "moov/mvhd",
   "trak/tkhd/edts/elst",
   "mdia/mdhd/hdlr",
   "minf/[x]mhd/dinf",
   "stbl",
   "stsd",
   "stts",
   "ctts",
   "stsc",
   "stsz",
   "stco|co64",
   "stss"
};

const char * const DebugBlockedCauseTable[] =
{
   "",
   "Insufficient Input Descriptors",      /* BMUXlib_File_MP4_P_BlockedCause_eInput */
   "Insufficient Output Descriptors",     /* BMUXlib_File_MP4_P_BlockedCause_eOutput */
   "Insufficient Box Buffer Space",       /* BMUXlib_File_MP4_P_BlockedCause_eBoxBuffer */
   "Insufficient Size Entries",           /* BMUXlib_File_MP4_P_BlockedCause_eSizeEntry */
   "Waiting for Output",                  /* BMUXlib_File_MP4_P_BlockedCause_eOutWait */
   "Insufficient Callback Data Entries"   /* BMUXlib_File_MP4_P_BlockedCause_eCallbackData */
};

static const char * const DebugDescriptorTagTable[] =
{
   "Unsupported",    /* 0x00 */
   "Unsupported",    /* 0x01 */
   "Unsupported",    /* 0x02 */
   "ES",             /* 0x03 - BMUXLIB_FILE_MP4_P_DESC_ES */
   "DecConfig",      /* 0x04 - BMUXLIB_FILE_MP4_P_DESC_DEC_CONF */
   "DecSpecific",    /* 0x05 - BMUXLIB_FILE_MP4_P_DESC_DEC_SPEC */
   "SLConfig",       /* 0x06 - BMUXLIB_FILE_MP4_P_DESC_SL_CONF */
   "Unsupported",    /* 0x07 */
   "Unsupported",    /* 0x08 */
   "Unsupported",    /* 0x09 */
   "Unsupported",    /* 0x0A */
   "Unsupported",    /* 0x0B */
   "Unsupported",    /* 0x0C */
   "Unsupported",    /* 0x0D */
   "MP4_ES_ID_Inc",  /* 0x0E - BMUXLIB_FILE_MP4_P_DESC_MP4_ES_ID_INC */
   "MP4_ES_ID_Ref",  /* 0x0F - BMUXLIB_FILE_MP4_P_DESC_MP4_ES_ID_REF */
   "MP4 IOD",        /* 0x10 - BMUXLIB_FILE_MP4_P_DESC_MP4_IOD */
   "MP4 OD"          /* 0x11 - BMUXLIB_FILE_MP4_P_DESC_MP4_OD */
};

#endif /* BDBG_DEBUG_BUILD */

/*************************
* P R I V A T E   A P I  *
**************************/

/*
   Function:
      BMUXlib_File_MP4_P_ProcessBoxes

   Create file header (ftyp), and any other boxes required.
   Also creates the mdat box on the appropriate mdat output interface
   in preparation for the mdat contents to be written

   After processing of input data is complete (i.e. all the metadata about
   the input is available), create the moov, and if necessary reposition
   the mdat as required for progressive-download compatibility

   Sets hMP4Mux->bHeaderDone when ftyp/mdat completed to indicate to DoMux()
   that it can move on with processing.

   NOTES:
   This MUST be state driven, since its not possible to complete this
   in one call to the Mux due to blocking caused by lack of resources (buffer
   space or descriptors).

   No other processing is done until BMUXlib_File_MP4_P_IsInputProcessingDone
   indicates success.

   Regardless of progressive-download compatibility (i.e. moov at
   start/end), the moov is always written to the final output storage

   Returns:
      BERR_SUCCESS           - if still processing
      BERR_NOT_SUPPORTED     - bad state detected
*/
BERR_Code BMUXlib_File_MP4_P_ProcessBoxes(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BERR_Code rc = BERR_SUCCESS;
   BMUXlib_File_MP4_P_BlockedCause eBlocked = BMUXlib_File_MP4_P_BlockedCause_eNone;
   BMUXlib_File_MP4_P_Output *pCurrentOutput;
   const BMUXlib_File_MP4_P_BoxInfo *pBoxInfo;
   BMUXlib_File_MP4_P_BoxBuffer *pBoxBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_RelocationBuffer *pRelBuffer = &hMP4Mux->stRelocationBuffer;
   BMUXlib_File_MP4_P_OutputCallbackData *pCallbackData;

   BDBG_ENTER(BMUXlib_File_MP4_P_ProcessBoxes);

   rc = BMUXlib_File_MP4_P_ProcessOutputDescriptorsCompleted(hMP4Mux);
   if (BERR_SUCCESS != rc)
   {
      BDBG_LEAVE(BMUXlib_File_MP4_P_ProcessBoxes);
      return rc;
   }

   BDBG_MODULE_MSG(BMUX_MP4_STATE, ("====Processing Boxes===="));

   while ((BMUXlib_File_MP4_P_BlockedCause_eNone == eBlocked)              /* we're not blocked ... */
           && (BMUXlib_File_MP4_P_BoxState_eDone != hMP4Mux->eBoxState)    /* we're not done ... */
           && (BERR_SUCCESS == rc))                                        /* and no error ... */
   {
      /* all boxes go to the main output, except mdat, which can go to either main or temporary output */
      pCurrentOutput = (BMUXlib_File_MP4_P_BoxType_eMdat == hMP4Mux->eCurrentBox)?
                           hMP4Mux->pMdatOutput: &hMP4Mux->aActiveOutputs[BMUXLIB_FILE_MP4_P_OUTPUT_MAIN];
      pBoxInfo = &BoxInfoTable[hMP4Mux->eCurrentBox];

      BDBG_MODULE_MSG(BMUX_MP4_STATE, ("Box: %s (State = %d)", DebugBoxTypeTable[hMP4Mux->eCurrentBox], hMP4Mux->eBoxState));

      switch(hMP4Mux->eBoxState)
      {
         case BMUXlib_File_MP4_P_BoxState_eCreateBox:
            /* create the fixed part of this box in memory */
            BDBG_MSG(("Creating %s @ offset: %lld", DebugBoxTypeTable[hMP4Mux->eCurrentBox], BMUXlib_Output_GetEndOffset(pCurrentOutput->hOutput)));

            if (!(*pBoxInfo->pfCreateBox)(hMP4Mux))
            {
               /* no space in box buffer: wait until descriptors consumed to free space in buffer */
               eBlocked = BMUXlib_File_MP4_P_BlockedCause_eBoxBuffer;
               break;
            }

            /* push the current offset of this box onto the offset stack ... */
            if (!OffsetStackPush(&hMP4Mux->stOffsetStack, BMUXlib_Output_GetEndOffset(pCurrentOutput->hOutput)))
            {
               /* NOTE: Offset stack is not a blocking resource - no stack space can be freed by waiting
                  - if we run out here, we need a bigger stack (boxes are nested too deep)!! */
               BDBG_ERR(("ProcessBoxes:: Offset Stack is full"));
               rc = BERR_TRACE(BERR_UNKNOWN);
               break;
            }

            /* now allocate the output descriptor for this box ... */
            hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAllocOutDescBox;
            break;

         case BMUXlib_File_MP4_P_BoxState_eAllocOutDescBox:
            {
               if (!BMUXlib_Output_IsSpaceAvailable(pCurrentOutput->hOutput))
               {
                  /* no descriptors available: wait and allocate the descriptors next time */
                  eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
                  break;
               }
               /* pass the mux handle to the callback */
               InitOutputCallback(hMP4Mux, BMUXlib_File_MP4_P_OutputCallback_eBoxBuffer, hMP4Mux);
               DebugSetDescriptorSource(pCurrentOutput, "Box");
               /* append box to the current output ... */
               BMUXlib_File_MP4_P_OutputDescriptorAppend(hMP4Mux, pCurrentOutput, pBoxBuffer->pBoxStart, NULL, 0, pBoxBuffer->uiPart1Size, BMUXlib_File_MP4_P_OutputCallback_eBoxBuffer);

               if (0 != pBoxBuffer->uiPart2Size)
                  /* box has a second part */
                  hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAllocOutDescBoxPart2;
               else
                  /* else, done with this box ... */
                  hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eDoneBox;

               break;
            }

         case BMUXlib_File_MP4_P_BoxState_eAllocOutDescBoxPart2:
            {
               if (!BMUXlib_Output_IsSpaceAvailable(pCurrentOutput->hOutput))
               {
                  /* no descriptors available: wait and allocate the descriptors next time */
                  eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
                  break;
               }
               /* pass mux handle to callback */
               InitOutputCallback(hMP4Mux, BMUXlib_File_MP4_P_OutputCallback_eBoxBuffer, hMP4Mux);
               DebugSetDescriptorSource(pCurrentOutput, "Box(2)");
               /* second half of box always starts at buffer base ... */
               BMUXlib_File_MP4_P_OutputDescriptorAppend(hMP4Mux, pCurrentOutput, pBoxBuffer->pBase, NULL, 0, pBoxBuffer->uiPart2Size, BMUXlib_File_MP4_P_OutputCallback_eBoxBuffer);

               /* done with this box ... */
               hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eDoneBox;
               break;
            }

         case BMUXlib_File_MP4_P_BoxState_eDoneBox:
            /* if box is mdat, special processing occurs to handle the "gap" in processing while
               input data processing completes ... */
            if (BMUXlib_File_MP4_P_BoxType_eMdat == hMP4Mux->eCurrentBox)
            {
               BDBG_MODULE_MSG(BMUX_MP4_STATE, ("Header is Done"));
               /* size update can't be completed until after all input processing is complete
                  thus, indicate headers are done, and wait ... */
               hMP4Mux->bHeaderDone = true;           /* indicate to DoMux() that input processing can begin */
               hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eDone;
               break;
               /* NOTE: this will return to a special state to finish processing of the mdat
                        (FinalizeMdat) - it will not continue to FinishBox. */
            }

            /* if there is content processing for this box, go to that state first ... */
            if (pBoxInfo->eContentState != BMUXlib_File_MP4_P_BoxState_eUnknown)
            {
               hMP4Mux->eBoxState = pBoxInfo->eContentState;
               break;
            }

            /* else, current box has been created;
               check if we need to nest boxes in this box ... */
            hMP4Mux->eBoxState = CheckForNestedBoxes(hMP4Mux);
            break;

         case BMUXlib_File_MP4_P_BoxState_eFinishBox:
            /* calculate the size of this box, and store it in the size buffer */
            {
               uint32_t *pSize = BMUXlib_File_MP4_P_FindFreeSizeEntry(&hMP4Mux->stSizes);
               uint32_t uiSize;
               uint64_t uiStartOffset;
               uint64_t uiCurrentOffset;
               if (NULL == pSize)
               {
                  /* no space available in the size buffer for the updated size
                     wait and store the size next time */
                  eBlocked = BMUXlib_File_MP4_P_BlockedCause_eSizeEntry;
                  break;
               }
               /* this must not pop the stack yet since the offset is required when
                  creating the output descriptor */
               if (!OffsetStackPeek(&hMP4Mux->stOffsetStack, &uiStartOffset))
               {
                  /* stack should not be empty! */
                  rc = BERR_TRACE(BERR_UNKNOWN);
                  break;
               }

               /* size is current offset - box start offset */
               uiCurrentOffset = BMUXlib_Output_GetEndOffset(pCurrentOutput->hOutput);
               uiSize = (uint32_t)(uiCurrentOffset - uiStartOffset);

               /* we do not support moov > 4Gb! Shouldn't happen - implies we need a largebox */
               if (0 != ((uiCurrentOffset - uiStartOffset) >> 32))
               {
                  BDBG_ERR(("Moov Box larger than 4Gb not supported - requires LargeBox"));
                  rc = BERR_TRACE(BERR_NOT_SUPPORTED);
                  break;
               }

               BDBG_MSG(("Finish %s: %d bytes", DebugBoxTypeTable[hMP4Mux->eCurrentBox], uiSize));
               /* update the size entry with the size of this box */
               BMUXlib_File_MP4_P_WriteU32BE(pSize, uiSize);

               /* if this is the stco box, keep its size and starting offset available
                  for adjustment of offsets if the mdat is relocated */
               if (hMP4Mux->bMoovAtStart && (BMUXlib_File_MP4_P_BoxType_eStco == hMP4Mux->eCurrentBox))
               {
                  /* NOTE: offset and size is referring to the table, excluding the header and entry-count */
                  hMP4Mux->pCurrentTrack->uiStcoOffset = uiStartOffset + (BMUXLIB_FILE_MP4_P_FULLBOX_HEADER_SIZE + sizeof(uint32_t));
                  hMP4Mux->pCurrentTrack->uiStcoSize = uiSize - (BMUXLIB_FILE_MP4_P_FULLBOX_HEADER_SIZE + sizeof(uint32_t));
               }
               /* if this is the end of the moov, and the moov is at the start, current location is the
                  new location of the mdat (used to adjust offsets in the stco) */
               if (hMP4Mux->bMoovAtStart && (BMUXlib_File_MP4_P_BoxType_eMoov == hMP4Mux->eCurrentBox))
               {
                  hMP4Mux->uiNewMdatOffset = (uint32_t)(uiCurrentOffset);
                  /* once moov is complete and we are relocating the mdat, if co64 box is not required
                     then verify that worst case adjusted offset can't be more than 32-bit size.
                     This would imply that we would need a co64 box, but we already created a stco!
                     In theory, this should not happen */
                  if (!hMP4Mux->bCo64Required)
                     if ((uiCurrentOffset + hMP4Mux->uiMdatSize) > 0xFFFFFFFF)
                     {
                        BDBG_ERR(("Adjusted Offsets due to relocation would exceed 32-bit values.  C064 box required"));
                        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
                        break;
                     }
               }
            }
            /* allocate a descriptor to send this size update to the output ... */
            /* FIXME: Is there a way to make this do in-place size update - how often will this occur?
               we would need to know the offset of the size, and whether that offset has been flushed to
               storage or not? Might be hard to know whether the offset in question has been written
               to storage already - we might be able to do this once we get the output to properly
               return its current EOF offset */
            hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAllocOutDescSize;
            break;

         case BMUXlib_File_MP4_P_BoxState_eAllocOutDescSize:
            {
               /* the following assumes that the updated size value is stored in the size buffer ... */
               uint64_t uiSizeOffset;        /* offset of the size field for the box to update */

               /* allocate an output descriptor for the updated size */
               if (!BMUXlib_Output_IsSpaceAvailable(pCurrentOutput->hOutput))
               {
                  /* no descriptors available: wait and allocate the descriptors next time */
                  eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
                  break;
               }

               pCallbackData = BMUXlib_File_MP4_P_NewOutputCallbackData(hMP4Mux, hMP4Mux->stSizes.pCurrentEntry, 0);
               if (NULL == pCallbackData)
               {
                  /* no more callback data entries available
                     (should not happen, since this implies there would be no output descriptors
                     available either) */
                  eBlocked = BMUXlib_File_MP4_P_BlockedCause_eCallbackData;
                  break;                                 /* wait until we have more available */
               }

               /* fetch the offset from the offset stack */
               if (!OffsetStackPop(&hMP4Mux->stOffsetStack, &uiSizeOffset))
               {
                  /* stack should not be empty! */
                  rc = BERR_TRACE(BERR_UNKNOWN);
                  break;
               }

               InitOutputCallback(hMP4Mux, BMUXlib_File_MP4_P_OutputCallback_eSizeBuffer, pCallbackData);
               DebugSetDescriptorSource(pCurrentOutput, "Box size entry");
               /* update the size entry for this box ... */
               BMUXlib_File_MP4_P_OutputDescriptorUpdate(hMP4Mux, pCurrentOutput, hMP4Mux->stSizes.pCurrentEntry, sizeof(uint32_t), uiSizeOffset, BMUXlib_File_MP4_P_OutputCallback_eSizeBuffer);

               /* we are done with this box ... */
               hMP4Mux->eBoxState = CheckForMoreBoxes(hMP4Mux);
            }
            break;

         /* the following state is where processing will return after input processing is complete ... */
         case BMUXlib_File_MP4_P_BoxState_eFinalizeMdat:
            /* update the mdat size ...
               (this may block due to lack of space in the box buffer for the mdat size) */
            {
               uint64_t uiMdatSize;
               uint64_t uiMdatStartOffset;
               bool bLargeSize;

               BDBG_MODULE_MSG(BMUX_MP4_STATE, ("====Update Mdat===="));

               /* this must not pop the stack yet incase the following requests for
                  storage fail - only pop the stack once successful */
               if (!OffsetStackPeek(&hMP4Mux->stOffsetStack, &uiMdatStartOffset))
               {
                  /* stack should not be empty! */
                  rc = BERR_TRACE(BERR_UNKNOWN);
                  break;
               }

               uiMdatSize = BMUXlib_Output_GetEndOffset(hMP4Mux->pMdatOutput->hOutput) - uiMdatStartOffset;
               if (uiMdatSize == 16)
               {
                  BDBG_ERR(("*******************************************************"));
                  BDBG_ERR(("* MDAT contains no data! Output is invalid/unplayable *"));
                  BDBG_ERR(("*******************************************************"));
               }
               bLargeSize = (0 != (uiMdatSize >> 32));

               /* NOTE: write the mdat size to the box buffer since its length can vary
                        (32 or 64 bits) ... i.e. it may be too big to fit in the size buffer */
               BoxBufferStartBox(pBoxBuffer);      /* size is effectively a new "box" */
               if (bLargeSize)
               {
                  /* write large size - offset is startoffset + 8 */
                  uiMdatStartOffset += 8;
                  /* see if there is space to store the size in the box buffer ... */
                  if (BoxBufferGetSpace(pBoxBuffer) < sizeof(uint64_t))
                  {
                     eBlocked = BMUXlib_File_MP4_P_BlockedCause_eBoxBuffer;
                     break;
                  }
#if BDBG_DEBUG_BUILD
                  pBoxBuffer->uiTotalBytesRequested += sizeof(uint64_t);
#endif
                  /* write the size to the box buffer ... */
                  BoxBufferWriteU64(pBoxBuffer, uiMdatSize);
               }
               else
               {
                  /* see if there is space to store the size in the box buffer ... */
                  if (BoxBufferGetSpace(pBoxBuffer) < sizeof(uint32_t))
                  {
                     eBlocked = BMUXlib_File_MP4_P_BlockedCause_eBoxBuffer;
                     break;
                  }
#if BDBG_DEBUG_BUILD
                  pBoxBuffer->uiTotalBytesRequested += sizeof(uint32_t);
#endif
                  /* write the size to the box buffer ... */
                  BoxBufferWriteU32(pBoxBuffer, (uint32_t)uiMdatSize);
               }
               BoxBufferFinishBox(pBoxBuffer);
               /* change the value on the top of the stack to the updated offset
                  of the size field - no need to check return code since we know
                  the stack is not empty */
               OffsetStackModify(&hMP4Mux->stOffsetStack, uiMdatStartOffset);
               hMP4Mux->uiMdatSize = uiMdatSize;
               BDBG_MSG(("Updating Mdat Size (%lld) at offset %lld", uiMdatSize, uiMdatStartOffset));
            }
            CalculateFinalizationSteps(hMP4Mux);
            hMP4Mux->pCurrentTrack = &hMP4Mux->aTracks[0];  /* start with the first track */
            hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAllocMdatSizeDesc;
            break;

         case BMUXlib_File_MP4_P_BoxState_eAllocMdatSizeDesc:
            {
               /* the following assumes that the updated size value is stored in the box buffer ... */
               uint64_t uiMdatSizeOffset;             /* offset of the mdat size field  */

               /* allocate an output descriptor for the updated size */
               if (!BMUXlib_Output_IsSpaceAvailable(pCurrentOutput->hOutput))
               {
                  /* no descriptors available: wait and allocate the descriptors next time */
                  eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
                  break;
               }
               /* fetch the offset of the mdat size field to update */
               if (!OffsetStackPop(&hMP4Mux->stOffsetStack, &uiMdatSizeOffset))
               {
                  /* stack should not be empty! */
                  rc = BERR_TRACE(BERR_UNKNOWN);
                  break;
               }

               /* pass the mux handle to the callback */
               InitOutputCallback(hMP4Mux, BMUXlib_File_MP4_P_OutputCallback_eBoxBuffer, hMP4Mux);
               DebugSetDescriptorSource(pCurrentOutput, "Box (mdat size)");
               /* write part1 of the mdat size to the first output descriptor ... */
               BMUXlib_File_MP4_P_OutputDescriptorUpdate(hMP4Mux, pCurrentOutput, pBoxBuffer->pBoxStart, pBoxBuffer->uiPart1Size, uiMdatSizeOffset, BMUXlib_File_MP4_P_OutputCallback_eBoxBuffer);

               if (0 != pBoxBuffer->uiPart2Size)
               {
                  /* box has a second part */
                  hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAllocMdatSizeDescPart2;
                  /* push new offset to the location of the second half
                     (no need to check return value since we know the stack is not full) */
                  OffsetStackPush(&hMP4Mux->stOffsetStack, uiMdatSizeOffset + pBoxBuffer->uiPart1Size);
               }
               else
               {
                  /* else, create the next box ... go on to create the moov */
                  hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eCreateMoov;
               }
               break;
            }

         case BMUXlib_File_MP4_P_BoxState_eAllocMdatSizeDescPart2:
            {
               /* the following assumes that the updated size value is stored in the box buffer ... */
               uint64_t uiMdatSizeOffset;             /* offset of the mdat size field  */

               /* allocate an output descriptor for part 2 of the updated size */
               if (!BMUXlib_Output_IsSpaceAvailable(pCurrentOutput->hOutput))
               {
                  /* no descriptors available: wait and allocate the descriptors next time */
                  eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
                  break;
               }
               /* fetch the offset of part 2 of the mdat size field to update */
               if (!OffsetStackPop(&hMP4Mux->stOffsetStack, &uiMdatSizeOffset))
               {
                  /* stack should not be empty! */
                  rc = BERR_TRACE(BERR_UNKNOWN);
                  break;
               }

               /* pass the mux handle to the callback */
               InitOutputCallback(hMP4Mux, BMUXlib_File_MP4_P_OutputCallback_eBoxBuffer, hMP4Mux);
               DebugSetDescriptorSource(pCurrentOutput, "Box (mdat size)(2)");
               /* write part 2 of the size update ... (second half of box always starts at buffer base) */
               BMUXlib_File_MP4_P_OutputDescriptorUpdate(hMP4Mux, pCurrentOutput, pBoxBuffer->pBase, pBoxBuffer->uiPart2Size, uiMdatSizeOffset, BMUXlib_File_MP4_P_OutputCallback_eBoxBuffer);

               /* create the next box ... go on to create the moov */
               hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eCreateMoov;
               break;
            }

         case BMUXlib_File_MP4_P_BoxState_eCreateMoov:
            {
               uint32_t uiTrackIndex;

               /* determine information needed for moov creation ... */
               for (uiTrackIndex = 0; uiTrackIndex < hMP4Mux->uiNumTracks; uiTrackIndex++)
               {
                  BMUXlib_File_MP4_P_TrackInfo *pTrack = &hMP4Mux->aTracks[uiTrackIndex];
                  /* if the input for this track is invalid, make the track invalid, and skip it */
                  if (NULL == pTrack->pInput)
                  {
                     /* this will ensure that this track is ignored ... */
                     pTrack->eType = BMUXlib_File_MP4_P_TrackType_eUnknown;
                     continue;
                  }
                  /* determine initial track offset ... */
                  if (NULL != pTrack->pInput && NULL != hMP4Mux->pLeadInput)
                     /* This track has an associated input, and the lead input was found,
                        therefore has an initial DTS ... */
                     /* NOTE: Here we are assuming the initial offset does not exceed 32 bits
                        (13 hours at 90kHz!!) */
                     pTrack->uiInitialOffset90kHz = pTrack->pInput->uiInitialDTS - hMP4Mux->pLeadInput->uiInitialDTS;
                  /* track duration is initial offset + duration of media */
                  pTrack->uiDuration90kHz += pTrack->uiInitialOffset90kHz;
                  /* determine presentation duration (90kHz) ... */
                  if (pTrack->uiDuration90kHz > hMP4Mux->uiPresentationDuration)
                     hMP4Mux->uiPresentationDuration = pTrack->uiDuration90kHz;
                  /* calculate average bitrate (NOTE: This will overflow if total bytes is > 23TB!! :) ... */
                  if (0 == pTrack->uiDuration90kHz)
                     pTrack->uiAvgBitrate = 0;
                  else
                     pTrack->uiAvgBitrate = (pTrack->uiTotalBytes * BMUXLIB_FILE_MP4_P_TIMESCALE_90KHZ * 8) / pTrack->uiDuration90kHz;
                  /* ensure that all the necessary metadata has been provided by the codecs ... */
                  VerifyTrackMetadata(pTrack);
                  BDBG_MSG(("Track %d: Bytes: %lld, Duration: %lld (90kHz), Avg Bitrate: %d, Initial Offset: %d",
                        pTrack->uiTrackID, pTrack->uiTotalBytes, pTrack->uiDuration90kHz, pTrack->uiAvgBitrate, pTrack->uiInitialOffset90kHz));

                  if (0 == pTrack->uiSampleCount)
                     BDBG_ERR(("Track %d: No Samples processed - output likely to be unplayable", uiTrackIndex));
               }
               /* skip any invalid tracks ... */
               while ((BMUXlib_File_MP4_P_TrackType_eUnknown == hMP4Mux->pCurrentTrack->eType)
                        && (hMP4Mux->pCurrentTrack != &hMP4Mux->aTracks[hMP4Mux->uiNumTracks]))
                  hMP4Mux->pCurrentTrack++;

               /* determine if its necessary to use co64 boxes instead of stco boxes ... */
               hMP4Mux->bCo64Required = (hMP4Mux->uiMdatSize > BMUXLIB_FILE_MP4_P_MAX_MDAT_FOR_STCO);

               /* if mdat is to be relocated, rewind the mdat storage ready to read the mdat back in ... */
               if (hMP4Mux->bMoovAtStart)
                  BMUXlib_Output_SetCurrentOffset(hMP4Mux->pMdatOutput->hOutput, 0, BMUXlib_Output_OffsetReference_eStart);

               hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eCreateBox;
               hMP4Mux->eCurrentBox = BoxSequenceTable[++hMP4Mux->uiBoxIndex];
               /* go create the moov ... */
               if (BMUXlib_File_MP4_P_BoxType_eMoov != hMP4Mux->eCurrentBox)
               {
                  BDBG_ERR(("Current Box is %s - should be creating Moov", DebugBoxTypeTable[hMP4Mux->eCurrentBox]));
                  rc = BERR_UNKNOWN;
                  break;
               }
               hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eCreateBox;
               hMP4Mux->stStatus.uiFinishStepsComplete++;
            }
            break;

         case BMUXlib_File_MP4_P_BoxState_eRetrieveMetadata:
            {
               bool bDataInCache;
               BMUXlib_File_MP4_P_MetadataType eMetadataType = BoxToMetadataTable[hMP4Mux->eCurrentBox];
               if (eMetadataType > BMUXlib_File_MP4_P_MetadataType_eMax)
               {
                  BDBG_ERR(("Attempt to retrieve metadata for a box that does not have metadata: %s", DebugBoxTypeTable[hMP4Mux->eCurrentBox]));
                  rc = BERR_TRACE(BERR_NOT_SUPPORTED);
                  break;
               }
               /* read the metadata appropriate for this box back into metadata cache so
                  that it can then be processed and output ...
                  (for example, if this box is the stss box, read the stss metadata back
                  from storage into the cache ready for transfer to the box output) */
               if (!BMUXlib_File_MP4_P_RetrieveMetadataToCache(hMP4Mux, eMetadataType, &bDataInCache))
               {
                  /* out of descriptors */
                  eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
                  break;
               }

               if (bDataInCache)
               {
                  /* data is already in the cache (can happen for run-length coded
                     metadata, for example) */
                  BMUXlib_File_MP4_P_ProcessMetadata(hMP4Mux);
                  hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAllocOutDescMetadata;
               }
               else
                  hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eWaitForRead;
            }
            break;

         case BMUXlib_File_MP4_P_BoxState_eWaitForRead:
            /* wait for the current read operation to complete before continuing
               Read operations are blocking since we need the data being returned before
               we can continue processing */
            if (BMUXlib_Output_IsDescriptorPendingCompletion(hMP4Mux->pCurrentMetadata->pOutput->hOutput))
            {
               eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutWait;
               break;
            }

            /* once read operation is complete, process the data returned, if necessary
               (for most sample table boxes, this is a "nop") */
            BMUXlib_File_MP4_P_ProcessMetadata(hMP4Mux);
            /* move on to create an output descriptor for this data ... */
            hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAllocOutDescMetadata;
            break;

         case BMUXlib_File_MP4_P_BoxState_eAllocOutDescMetadata:
            {
               BMUXlib_File_MP4_P_MetadataInterface *pMetadata = hMP4Mux->pCurrentMetadata;
               BMUXlib_File_MP4_P_MetadataCache *pCache = &pMetadata->stCache;
               uint32_t uiLength;
               uint32_t uiEntrySize = pCache->uiEntrySize;

               /* allocate an output descriptor for writing the metadata for this box ... */
               if (!BMUXlib_Output_IsSpaceAvailable(pCurrentOutput->hOutput))
               {
                  /* no descriptors available: wait and allocate the descriptors next time */
                  eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
                  break;
               }
               /* determine the amount of data to write (number of entries is always the
                  current write index (since metadata is written from the start of the buffer)
                  so data size is num entries * entry size */
               if ((hMP4Mux->eCurrentBox == BMUXlib_File_MP4_P_BoxType_eStco) & !hMP4Mux->bCo64Required)
                  uiEntrySize = sizeof(uint32_t);        /* stco entries are compacted */
               uiLength = pCache->uiWriteIndex * uiEntrySize;

               /* callback to "free" this cache entry */
               InitOutputCallback(hMP4Mux, BMUXlib_File_MP4_P_OutputCallback_eMetadataCache, pCache);
               DebugSetDescriptorSource(pCurrentOutput, "Metadata");
               /* Append the metadata to the box */
               BMUXlib_File_MP4_P_OutputDescriptorAppend(hMP4Mux, pCurrentOutput, pCache->pBuffer, NULL, 0, uiLength, BMUXlib_File_MP4_P_OutputCallback_eMetadataCache);

               /* we are done with this box ... */
               hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eWaitForWrite;
            }
            break;

         case BMUXlib_File_MP4_P_BoxState_eWaitForWrite:
            /* check if there are more entries to process for this metadata
               if so, we must wait for the previous write to complete,
               else, we can continue to the next read, since it does not use
               the same cache
            */
            if (0 != hMP4Mux->pCurrentMetadata->uiEntryCount)
            {
               /* since the write of metadata to a box uses the metadata cache, we must
                  wait for the data to be written to storage before we can re-use the
                  cache to read in more data.  Therefore pend on the current output */
               if (BMUXlib_Output_IsDescriptorPendingCompletion(pCurrentOutput->hOutput))
               {
                  eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutWait;
                  break;
               }
               /* go back and retrieve the remaining entries ... */
               hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eRetrieveMetadata;
            }
            else
            {
               /* no more metadata to process ... */
               /* this box has been created, so check for nested boxes ... */
               hMP4Mux->eBoxState = CheckForNestedBoxes(hMP4Mux);
               /* NOTE: here we only count the processing of this box's metadata as a whole
                  unit, not each block retrieved - thus we pass thru here once per metadata
                  box per trak  */
               hMP4Mux->stStatus.uiFinishStepsComplete++;
            }
            break;

         case BMUXlib_File_MP4_P_BoxState_eRelocateMdatRead:
         {
            uint32_t uiLength;

            /* read a chunk of the mdat into internal storage */
            if (!BMUXlib_Output_IsSpaceAvailable(hMP4Mux->pMdatOutput->hOutput))
            {
               /* no descriptors available: wait and allocate the descriptors next time */
               eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
               break;
            }
            /* create a metadata descriptor to read the mdat chunk back from storage into memory */
            /* NOTE: The data is always written to the start of the buffer */
            uiLength = (hMP4Mux->uiMdatSize > pRelBuffer->uiSize)?pRelBuffer->uiSize:hMP4Mux->uiMdatSize;
            pRelBuffer->uiBytesUsed = uiLength;

            DebugSetDescriptorSource(hMP4Mux->pMdatOutput, "Mdat");
            /* read back the mdat chunk into the relocation buffer ... */
            /* NOTE: No callback needed for relocation buffer ... */
            BMUXlib_File_MP4_P_OutputDescriptorRead(hMP4Mux, hMP4Mux->pMdatOutput, pRelBuffer->pBase, uiLength, BMUXlib_File_MP4_P_OutputCallback_eNone);
            hMP4Mux->uiMdatSize -= uiLength;
            hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eRelocateMdatWaitForRead;
            break;
         }

         case BMUXlib_File_MP4_P_BoxState_eRelocateMdatWaitForRead:
            /* wait for the mdat chunk to be read */
            /* Read operations are blocking since we need the data being returned before
               we can continue processing, therefore pend on the mdat output */
            if (BMUXlib_Output_IsDescriptorPendingCompletion(hMP4Mux->pMdatOutput->hOutput))
            {
               eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutWait;
               break;
            }

            /* move on to create an output descriptor for this data ... */
            hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eRelocateMdatWrite;
            /* count mdat relocation read steps ... */
            hMP4Mux->stStatus.uiFinishStepsComplete++;
            break;

         case BMUXlib_File_MP4_P_BoxState_eRelocateMdatWrite:
         {
            /* write the mdat chunk to its new position */
            uint32_t uiLength = pRelBuffer->uiBytesUsed;

            /* allocate an output descriptor for writing the metadata for this box ... */
            if (!BMUXlib_Output_IsSpaceAvailable(pCurrentOutput->hOutput))
            {
               /* no descriptors available: wait and allocate the descriptors next time */
               eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
               break;
            }
            /* create a descriptor to process the mdat chunk */
            DebugSetDescriptorSource(pCurrentOutput, "Mdat");
            /* NOTE: no callback needed for relocation buffer */
            BMUXlib_File_MP4_P_OutputDescriptorAppend(hMP4Mux, pCurrentOutput, pRelBuffer->pBase, NULL, 0, uiLength, BMUXlib_File_MP4_P_OutputCallback_eNone);
            /* we are done with this chunk ... */
            hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eRelocateMdatWaitForWrite;
            break;
         }

         case BMUXlib_File_MP4_P_BoxState_eRelocateMdatWaitForWrite:
            /* wait for write to complete (mdat is being written to the current output) */
            if (BMUXlib_Output_IsDescriptorPendingCompletion(pCurrentOutput->hOutput))
            {
               eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutWait;
               break;
            }

            /* if mdat remaining, go back read more */
            if (0 != hMP4Mux->uiMdatSize)
               hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eRelocateMdatRead;
            else
            {
               /* else, adjust offsets in the stco to compensate for new location of mdat */
               /* start with the first trak ... */
               hMP4Mux->pCurrentTrack = &hMP4Mux->aTracks[0];  /* start with the first track */
               /* skip any invalid tracks ... */
               while ((BMUXlib_File_MP4_P_TrackType_eUnknown == hMP4Mux->pCurrentTrack->eType)
                        && (hMP4Mux->pCurrentTrack != &hMP4Mux->aTracks[hMP4Mux->uiNumTracks]))
                  hMP4Mux->pCurrentTrack++;
               /* rewind output storage to location of the stco for the first track ... */
               BMUXlib_Output_SetCurrentOffset(pCurrentOutput->hOutput, hMP4Mux->pCurrentTrack->uiStcoOffset, BMUXlib_Output_OffsetReference_eStart);
               hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsRead;
            }
            /* count mdat relocation write steps ... */
            hMP4Mux->stStatus.uiFinishStepsComplete++;
            break;

         case BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsRead:
         {
            /* read a chunk of the stco back into relocation buffer
               (since we are reading from the moov, we read from the main output storage) */
            uint32_t uiLength;

            /* fetch an output descriptor for this read transaction */
            if (!BMUXlib_Output_IsSpaceAvailable(pCurrentOutput->hOutput))
            {
               /* no descriptors available: wait and allocate the descriptors next time */
              eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
            }
            /* create a metadata descriptor to read the chunk of stco/co64 data back into relocation buffer */
            /* NOTE: The data is always written to the start of the buffer */
            uiLength = (hMP4Mux->pCurrentTrack->uiStcoSize > pRelBuffer->uiSize)?pRelBuffer->uiSize:hMP4Mux->pCurrentTrack->uiStcoSize;
            /* truncate the length to ensure its a multiple of an entry size ... */
            if (hMP4Mux->bCo64Required)
               uiLength &= ~7;     /* 64-bit entries (i.e. each entry is 8 bytes) */
            else
               uiLength &= ~3;     /* 32-bit entries (i.e. each entry is 4 bytes) */
            pRelBuffer->uiBytesUsed = uiLength;
            /* we need to read back the stco/co64 ... */
            DebugSetDescriptorSource(pCurrentOutput, "Offsets");
            /* NOTE: no callback needed for relocation buffer */
            BMUXlib_File_MP4_P_OutputDescriptorRead(hMP4Mux, pCurrentOutput, pRelBuffer->pBase, uiLength, BMUXlib_File_MP4_P_OutputCallback_eNone);
            /* NOTE: Do not update the offset, since we need to write back to the same location */
            hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsWaitForRead;
            break;
         }

         case BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsWaitForRead:

            /* wait for read to complete ... */
            /* Read operations are blocking since we need the data being returned before
               we can continue processing (offsets are read back from current output) */
            if (BMUXlib_Output_IsDescriptorPendingCompletion(pCurrentOutput->hOutput))
            {
               eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutWait;
               break;
            }
            /* ... then adjust the offsets based on new mdat location */
            BMUXlib_File_MP4_P_AdjustOffsets(hMP4Mux);
            /* move on to create an output descriptor for the adjusted stco data ... */
            hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsWrite;
            /* count offset adjustment read steps ... */
            hMP4Mux->stStatus.uiFinishStepsComplete++;
            break;

         case BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsWrite:
         {
            /* write the adjusted stco metdata back to disk */
            uint32_t uiLength = pRelBuffer->uiBytesUsed;

            /* allocate an output descriptor for writing the adjusted offsets ... */
            if (!BMUXlib_Output_IsSpaceAvailable(pCurrentOutput->hOutput))
            {
               /* no descriptors available: wait and allocate the descriptors next time */
               eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutput;
               break;
            }
             /* write stco/co64 entries back to the same location in the current output (i.e. go back to where we read from)
                NOTE: this is an offset from the *current* location, but since we can't do negative offsets, simply
                do an absolute offset */
            DebugSetDescriptorSource(pCurrentOutput, "Offsets");
            /* NOTE: no callback needed for relocation buffer */
            BMUXlib_File_MP4_P_OutputDescriptorUpdate(hMP4Mux, pCurrentOutput, pRelBuffer->pBase, uiLength, BMUXlib_Output_GetCurrentOffset(pCurrentOutput->hOutput) - uiLength, BMUXlib_File_MP4_P_OutputCallback_eNone);

            hMP4Mux->pCurrentTrack->uiStcoSize -= uiLength;
            /* we are done with this chunk ... */
            hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsWaitForWrite;
            break;
         }

         case BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsWaitForWrite:
            /* wait for the write to complete (offsets are written back to current output) */
            if (BMUXlib_Output_IsDescriptorPendingCompletion(pCurrentOutput->hOutput))
            {
               eBlocked = BMUXlib_File_MP4_P_BlockedCause_eOutWait;
               break;
            }

            /* if stco/co64 entries remaining, go back read more */
            if (0 != hMP4Mux->pCurrentTrack->uiStcoSize)
               hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsRead;
            else
            {
               /* go to the next track ... */
               hMP4Mux->pCurrentTrack++;
               /* skip any invalid tracks ... */
               while ((hMP4Mux->pCurrentTrack != &hMP4Mux->aTracks[hMP4Mux->uiNumTracks])
                  && (BMUXlib_File_MP4_P_TrackType_eUnknown == hMP4Mux->pCurrentTrack->eType))
                     hMP4Mux->pCurrentTrack++;
               if (hMP4Mux->pCurrentTrack != &hMP4Mux->aTracks[hMP4Mux->uiNumTracks])
               {
                  /* more tracks to process ... */
                  /* rewind output storage to location of the stco for this track ... */
                  BMUXlib_Output_SetCurrentOffset(pCurrentOutput->hOutput, hMP4Mux->pCurrentTrack->uiStcoOffset, BMUXlib_Output_OffsetReference_eStart);
                  hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eAdjustOffsetsRead;
               }
               else
                  hMP4Mux->eBoxState = BMUXlib_File_MP4_P_BoxState_eDone;
            }
            /* count offset adjustment write steps ... */
            hMP4Mux->stStatus.uiFinishStepsComplete++;
            break;

         case BMUXlib_File_MP4_P_BoxState_eDone:
            /* finished! do nothing - in theory we should only come here once!
               (main loop should then detect "done" and stop calling) */
            break;

         default:
            /* Unknown state! should not happen ... typically indicates memory overwrite */
            BDBG_ERR(("ProcessBoxes:: Unknown State: %d", hMP4Mux->eBoxState));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            break;
      } /* end: switch state */
   } /* end: while not blocked and not done and not error */

   if (BMUXlib_File_MP4_P_BlockedCause_eNone != eBlocked)
      BDBG_MODULE_MSG(BMUX_MP4_STATE, ("Process Boxes: Blocked: %s", DebugBlockedCauseTable[eBlocked]));

   if (BERR_SUCCESS == rc)
      rc = BMUXlib_File_MP4_P_ProcessOutputDescriptorsWaiting(hMP4Mux);

   BDBG_LEAVE(BMUXlib_File_MP4_P_ProcessBoxes);
   return rc;
}

/* Free Box information referenced by the current output descriptor
   NOTE: This assumes that the data is consumed and freed in the
   order it was created
*/
void BMUXlib_File_MP4_P_FreeBoxBufferData(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint8_t *pData, uint32_t uiSize)
{
#ifndef BDBG_DEBUG_BUILD
   BSTD_UNUSED(pData);
#endif
   BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER, ("Freeing %d bytes from address %p", uiSize, pData));

   /* move read pointer up by the size */
   pBuffer->pRead += uiSize;
   if (pBuffer->pRead >= pBuffer->pEnd)
      pBuffer->pRead = pBuffer->pBase;
   BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER, ("New Read location = %p", pBuffer->pRead));
}

/***********************************
* S T A T I C   F U N C T I O N S  *
***********************************/

/* verify that the necessary metadata has been provided by the codecs, and if not set the
   relevant defaults */
static void VerifyTrackMetadata(BMUXlib_File_MP4_P_TrackInfo *pTrack)
{
   switch (pTrack->eType)
   {
      case BMUXlib_File_MP4_P_TrackType_eAudio:
         /* check dec spec info ... */
         /* the currently supported codecs expect decoder-specific info ... */
         if (0 == pTrack->uiDecSpecInfoSize)
         {
            /* this is unrecoverable, since we have no way of knowing what to put here */
            BDBG_ERR(("Audio Decoder Specific Information not set. Audio output is likely to be unplayable"));
         }
         if (0 == pTrack->uiDecodeBufferSize)
         {
            BDBG_WRN(("Audio Metadata (HRD Buffer Size) not set - using default: %d. Output may be incompatible with some players.", pTrack->uiMaxSampleSize * 2));
            pTrack->uiDecodeBufferSize = pTrack->uiMaxSampleSize * 2;
         }
         if (0 == pTrack->uiSampleRate)
         {
            BDBG_WRN(("Audio Metadata (Sample Rate) not set - using default: %d. Output may be incompatible with some players.", BMUXLIB_FILE_MP4_P_DEFAULT_AUDIO_SAMPLE_RATE));
            pTrack->uiSampleRate = BMUXLIB_FILE_MP4_P_DEFAULT_AUDIO_SAMPLE_RATE;
         }
         if (0 == pTrack->uiMaxBitrate)
         {
            BDBG_WRN(("Audio Metadata (Max. Bitrate) not set - Using Avg Bitrate (%d). Output may be incompatible with some players.", pTrack->uiAvgBitrate));
            pTrack->uiMaxBitrate = pTrack->uiAvgBitrate;
         }
         break;

      case BMUXlib_File_MP4_P_TrackType_eVideo:
         /* most of these metadata settings are generic for all video codecs ... */
         if (0 == pTrack->uiWidth)
         {
            BDBG_WRN(("Video Metadata (Width) not set - using default: %d. Output may be incompatible with some players.", BMUXLIB_FILE_MP4_P_DEFAULT_VIDEO_WIDTH));
            pTrack->uiWidth = BMUXLIB_FILE_MP4_P_DEFAULT_VIDEO_WIDTH;
         }
         if (0 == pTrack->uiHeight)
         {
            BDBG_WRN(("Video Metadata (Height) not set - using default: %d. Output may be incompatible with some players.", BMUXLIB_FILE_MP4_P_DEFAULT_VIDEO_HEIGHT));
            pTrack->uiHeight = BMUXLIB_FILE_MP4_P_DEFAULT_VIDEO_HEIGHT;
         }
         if (0 == pTrack->uiDecodeBufferSize)
         {
            BDBG_WRN(("Video Metadata (HRD Buffer Size) not set - using default: %d. Output may be incompatible with some players.", BMUXLIB_FILE_MP4_P_DEFAULT_VIDEO_BUFSIZE));
            pTrack->uiDecodeBufferSize = BMUXLIB_FILE_MP4_P_DEFAULT_VIDEO_BUFSIZE;
         }
         if (0 == pTrack->uiMaxBitrate)
         {
            BDBG_WRN(("Video Metadata (Max. Bitrate) not set - Using Avg Bitrate (%d). Output may be incompaible with some players.", pTrack->uiAvgBitrate));
            pTrack->uiMaxBitrate = pTrack->uiAvgBitrate;
         }
         /* we don't check frame rate code, since it has no use unless it is provided prior to Start() */
         break;

      default:
         /* do nothing for other input/track types (no metadata expected) */
         break;
   } /* end: switch track type */
}

/* estimate required finalization steps for display of % complete ...
   need: 1 for Mdat size update / per-track metadata
         per-track:
           1 for stts
           1 for ctts (if required)
           1 for stsz
           1 for stco/c064
           1 for stss (if required)
         if PDL:
           1 mdat relocation read
           1 mdat relocation write
                => per iteration, where number iterations = round up (mdat size / relocation buffer size)
           1 offset adjust read
           1 offset adjust write
                => per iteration per track, where number of iterations = stco entry count for this track * entry size / relocation buffer size
*/
static void CalculateFinalizationSteps(BMUXlib_File_MP4_Handle hMP4Mux)
{
   uint32_t uiTrackIndex;

   for (uiTrackIndex = 0; uiTrackIndex < hMP4Mux->uiNumTracks; uiTrackIndex++)
   {
      BMUXlib_File_MP4_P_TrackInfo *pTrack = &hMP4Mux->aTracks[uiTrackIndex];
      /* if this track is invalid skip it */
      if (BMUXlib_File_MP4_P_TrackType_eUnknown == pTrack->eType)
         continue;
      hMP4Mux->stStatus.uiFinishStepsTotal += 3;   /* stco/stsz/stts metadata processing steps */
      if (pTrack->bCttsRequired)
         hMP4Mux->stStatus.uiFinishStepsTotal++;   /* + ctts */
      if (pTrack->bStssRequired)
         hMP4Mux->stStatus.uiFinishStepsTotal++;   /* + stss */
   }
   if (hMP4Mux->bMoovAtStart)
   {
      uint32_t uiNumIterations = (hMP4Mux->uiMdatSize + hMP4Mux->stRelocationBuffer.uiSize) / hMP4Mux->stRelocationBuffer.uiSize;
      hMP4Mux->stStatus.uiFinishStepsTotal += (uiNumIterations * 2);
      for (uiTrackIndex = 0; uiTrackIndex < hMP4Mux->uiNumTracks; uiTrackIndex++)
      {
         BMUXlib_File_MP4_P_TrackInfo *pTrack = &hMP4Mux->aTracks[uiTrackIndex];
         uint32_t uiEntrySize = (hMP4Mux->bCo64Required)?sizeof(BMUXlib_File_MP4_SampleOffset):(sizeof(BMUXlib_File_MP4_SampleOffset)/2);
         uint32_t uiEntriesPerBuffer = hMP4Mux->stRelocationBuffer.uiSize / uiEntrySize;
         /* if this track is invalid skip it */
         if (BMUXlib_File_MP4_P_TrackType_eUnknown == pTrack->eType)
            continue;
         /* the following assumes one sample per chunk (i.e. stco is giving sample offsets)*/
         uiNumIterations = (pTrack->uiSampleCount + uiEntriesPerBuffer) / uiEntriesPerBuffer;
         hMP4Mux->stStatus.uiFinishStepsTotal += (uiNumIterations * 2);
      }
   }
}

/************************
     Box Processing
************************/
static BMUXlib_File_MP4_P_BoxState CheckForNestedBoxes(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxType eNextBox = BoxSequenceTable[hMP4Mux->uiBoxIndex+1];
   const BMUXlib_File_MP4_P_BoxInfo *pBoxInfo = &BoxInfoTable[hMP4Mux->eCurrentBox];
   const BMUXlib_File_MP4_P_BoxInfo *pNextBoxInfo = &BoxInfoTable[eNextBox];

   BDBG_MODULE_MSG(BMUX_MP4_STATE, ("Check for Nested boxes: Next Box in Sequence is %s", DebugBoxTypeTable[eNextBox]));
   /* skip optional boxes ...
      (does not matter whether the skipped box is nested -
      its the next non-skipped box that determines what to do next) */
   while ((BMUXlib_File_MP4_P_BoxType_eUnknown != eNextBox)
         && !(*pNextBoxInfo->pfIsRequired)(hMP4Mux))
   {
      BDBG_MSG(("Skipping %s - not required", DebugBoxTypeTable[eNextBox]));
      hMP4Mux->uiBoxIndex++;
      eNextBox = BoxSequenceTable[hMP4Mux->uiBoxIndex+1];
      pNextBoxInfo = &BoxInfoTable[eNextBox];
   }

   if (BMUXlib_File_MP4_P_BoxType_eUnknown != eNextBox)
   {
      /* there are more boxes in the sequence .. */
      /* check if the next box is a nested box ... */
      if (pNextBoxInfo->uiNestingLevel > pBoxInfo->uiNestingLevel)
      {
         BMUXlib_File_MP4_P_BoxStack *pBoxStack = &hMP4Mux->stBoxStack;
         /* if so, push current box onto Stack, move to next box */
         /* check if stack full (i.e. max nesting level exceeded)
            - should not happen if box info nesting levels are correct */
         BDBG_ASSERT(pBoxStack->uiTopStack != BMUXlib_File_MP4_P_BoxNesting_eMax);
         BDBG_MODULE_MSG(BMUX_MP4_STACK, ("***Pushing Box %s (nest level %d)***", DebugBoxTypeTable[hMP4Mux->eCurrentBox], pBoxStack->uiTopStack));
         pBoxStack->aData[pBoxStack->uiTopStack++] = hMP4Mux->eCurrentBox;
         hMP4Mux->uiBoxIndex++;
         hMP4Mux->eCurrentBox = eNextBox;
         return BMUXlib_File_MP4_P_BoxState_eCreateBox;
      }
   }
   /* else, finish the current box (update the size) */
   return BMUXlib_File_MP4_P_BoxState_eFinishBox;
}

/* Check if there are more boxes to be placed at this nesting level */
static BMUXlib_File_MP4_P_BoxState CheckForMoreBoxes(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxType eNextBox = BoxSequenceTable[hMP4Mux->uiBoxIndex+1];
   const BMUXlib_File_MP4_P_BoxInfo *pNextBoxInfo = &BoxInfoTable[eNextBox];
   const BMUXlib_File_MP4_P_BoxInfo *pBoxInfo = &BoxInfoTable[hMP4Mux->eCurrentBox];
   BMUXlib_File_MP4_P_BoxStack *pBoxStack = &hMP4Mux->stBoxStack;

   BDBG_MODULE_MSG(BMUX_MP4_STATE, ("Check for more Boxes: Next Box in Sequence is %s", DebugBoxTypeTable[eNextBox]));
   /* if current box is a box trak, see if more traks (stay at the same nesting level) */
   if (BMUXlib_File_MP4_P_BoxType_eTrak == hMP4Mux->eCurrentBox)
   {
      /* see if there are more tracks to process ... */
      hMP4Mux->pCurrentTrack++;
      /* skip any invalid tracks ... */
      while ((BMUXlib_File_MP4_P_TrackType_eUnknown == hMP4Mux->pCurrentTrack->eType)
         && (hMP4Mux->pCurrentTrack != &hMP4Mux->aTracks[hMP4Mux->uiNumTracks]))
         hMP4Mux->pCurrentTrack++;
      if (hMP4Mux->pCurrentTrack != &hMP4Mux->aTracks[hMP4Mux->uiNumTracks])
      {
         /* if so, return to trak in the sequence */
         hMP4Mux->uiBoxIndex = BMUXLIB_FILE_MP4_P_SEQUENCE_INDEX_TRAK;
         /* and create a new trak box */
         return BMUXlib_File_MP4_P_BoxState_eCreateBox;
      }
      /* else, no more tracks, so see if other nested boxes at this level */
   }

   /* check if there are more boxes to place in this box (at the same nesting level) ... */
   /* BE CAREFUL, this must NOT increment the sequence unless the box is optional, or it is at the
      same level - if different level, then we finish the container */

   /* skip any boxes that are not required until we see a box that is
      required and is at the same nesting level, and is not end-of-sequence
      (if the unneeded box is at a different nesting level then we must
       move on to process the container box - do not skip this box) ... */
   while ((BMUXlib_File_MP4_P_BoxType_eUnknown != eNextBox)
         && (pNextBoxInfo->uiNestingLevel == pBoxInfo->uiNestingLevel)
         && !(*pNextBoxInfo->pfIsRequired)(hMP4Mux))
   {
      BDBG_MSG(("Skipping %s - not required", DebugBoxTypeTable[eNextBox]));
      hMP4Mux->uiBoxIndex++;
      eNextBox = BoxSequenceTable[hMP4Mux->uiBoxIndex+1];
      pNextBoxInfo = &BoxInfoTable[eNextBox];
   }

   /* ensure we didn't hit the end of the sequence ... */
   if (BMUXlib_File_MP4_P_BoxType_eUnknown != eNextBox)
   {
      if (pNextBoxInfo->uiNestingLevel == pBoxInfo->uiNestingLevel)
      {
         /* more boxes to be placed in this container ... go to next box */
         hMP4Mux->uiBoxIndex++;
         hMP4Mux->eCurrentBox = eNextBox;
         return BMUXlib_File_MP4_P_BoxState_eCreateBox;
      }
   }

   /* if no more boxes to be placed at this level, return to container box ...*/
   if (0 == pBoxStack->uiTopStack)
   {
      /* no more boxes on the stack, we are done with boxes ... */
      return (hMP4Mux->bMoovAtStart)?
                              BMUXlib_File_MP4_P_BoxState_eRelocateMdatRead:
                              BMUXlib_File_MP4_P_BoxState_eDone; /* moov is already at the end */
   }
   /* else, update size of the container next ... */
   hMP4Mux->eCurrentBox = pBoxStack->aData[--pBoxStack->uiTopStack];
   BDBG_MODULE_MSG(BMUX_MP4_STACK, ("***Popping Box %s (nest level = %d)***", DebugBoxTypeTable[hMP4Mux->eCurrentBox], pBoxStack->uiTopStack));
   /* do not change the sequence, yet - done if more boxes to be placed at this level */
   return BMUXlib_File_MP4_P_BoxState_eFinishBox;
}

/*********************
     Box Creation
*********************/

/*
   Basic predicate to determine if the CTTS box is required for the current
   track being processed
*/
static bool IsCttsRequired(BMUXlib_File_MP4_Handle hMP4Mux)
{
   return hMP4Mux->pCurrentTrack->bCttsRequired;
}

/*
   Basic predicate to determine if the STSS box is required for the current
   track being processed.
*/
static bool IsStssRequired(BMUXlib_File_MP4_Handle hMP4Mux)
{
   return hMP4Mux->pCurrentTrack->bStssRequired;
}

/*
   Dummy "predicate" to indicate the current/next box is always required
*/
static bool IsAlwaysRequired(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BSTD_UNUSED(hMP4Mux);
   return true;
}

/*
   Notes on Box Creation
   ---------------------
   Two types of boxes are handled:
   * fixed-size: the size of the box is known in advance
   * unbounded: the size of the box is not known in advance, and
                must be subsequently determined and updated in the
                output stream.

   BoxBufferStartBox() is used to indicate that a new unbounded
   box is being started, that may or may not contain other unbounded
   boxes or fixed size boxes.

   If a fixed size box is contained within other boxes, then the
   PL size MUST be correct for the expected contents, and
   BoxBufferStartBox() is NOT used.

   The PL size specified to CreateBox() and its variants is used to
   determine if there is room in the buffer for the box(es), and so
   this value must be accurate.

   For boxes that are unbounded, they have an unknown size (their
   PL size uses BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN unless some fixed
   part is added in which case the PL Size indicates the size of the
   fixed part).

   For each unbounded box that incorporates BoxBufferStartBox
   there must be a corresponding entry in the box sequence table!

   All other boxes that are contained within one of these boxes
   must not have a BoxBufferStartBox() call before the
   CreateBox...() call.

   *** IMPORTANT ***
   Do not modify the contents of boxes without first updating
   the PL Sizes in the include file: THEY MUST MATCH
*/

/*
   Create main file type box

   We support only mp4 file (ISO/IEC-14496-14:2003, version 2),
   so compatible types are mp42, mp41, and isom

   NOTE: if none of the additional features of H.264 are utilized,
   as described in Section 4 of ISO/IEC-14496-15, then brand "avc1" is
   not required - currently, we DO NOT utilize those features.

*/
static bool CreateBoxFtyp(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   BoxBufferStartBox(pBuffer);
   if (!CreateBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_FTYP, BMUXLIB_FILE_MP4_P_PLSIZE_FTYP))
      return false;        /* no room in buffer for the box */
   BoxBufferWriteFourCC(pBuffer, BMUXLIB_FILE_MP4_P_BRAND_MP42);        /* major brand */
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_FTYP_MINOR_VER);       /* minor version */
   /* compatibility brands
      NOTE: DO NOT add more brands without first adjusting the box size */
   BoxBufferWriteFourCC(pBuffer, BMUXLIB_FILE_MP4_P_BRAND_MP42);
   BoxBufferWriteFourCC(pBuffer, BMUXLIB_FILE_MP4_P_BRAND_MP41);
   BoxBufferWriteFourCC(pBuffer, BMUXLIB_FILE_MP4_P_BRAND_ISOM);
   BoxBufferFinishBox(pBuffer);
   return true;
}

/* create the header for the mdat box
   Note that a large box is written to allow mdat size to exceed 4Gb.
   If it is determined that large box is not needed, the actual mdat size
   could be written to the size field and the 8 bytes that follow the
   box type would be "dead" space (unused - i.e. no metadata would refer
   to this data)
*/
static bool CreateBoxMdat(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   BoxBufferStartBox(pBuffer);
   if (!CreateLargeBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_MDAT, BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN))
      return false;
   BoxBufferFinishBox(pBuffer);
   /* no other data to write for this box (created by processing inputs) */
   return true;
}

static bool CreateBoxMoov(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   BoxBufferStartBox(pBuffer);
   if (!CreateBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_MOOV, BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN))
      return false;        /* no room in buffer for the box */

   if (!CreateBoxMvhd(hMP4Mux))
      return false;
   BoxBufferFinishBox(pBuffer);
   return true;
}

static bool CreateBoxMvhd(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   uint8_t uiBoxVersion = (hMP4Mux->uiPresentationDuration > 0xFFFFFFFF)?
                              BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1:  /* use a 64-bit fields if duration requires it */
                              BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0;
   uint32_t uiPLSize = (BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1 == uiBoxVersion)?
                     BMUXLIB_FILE_MP4_P_PLSIZE_MVHD_1: BMUXLIB_FILE_MP4_P_PLSIZE_MVHD_0;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_MVHD, uiPLSize,
            uiBoxVersion, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   if (uiBoxVersion == BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1)
   {
      BoxBufferWriteU64(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* creation_time */
      BoxBufferWriteU64(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* modification time */
      /* timescale is always 90kHz since this is the inherent timing of the presentation ... */
      BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_TIMESCALE_90KHZ);
      BoxBufferWriteU64(pBuffer, hMP4Mux->uiPresentationDuration);      /* duration (in 90kHz timescale) */
   }
   else
   {
      BoxBufferWriteU32(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* creation_time */
      BoxBufferWriteU32(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* modification time */
      /* timescale is always 90kHz since this is the inherent timing of the presentation ... */
      BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_TIMESCALE_90KHZ);
      BoxBufferWriteU32(pBuffer, hMP4Mux->uiPresentationDuration);      /* duration (in 90kHz timescale) */
   }

   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RATE_1_0);    /* = 0x00010000 in 16.16 fixed = 1.0 (normal playback rate) */
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_VOLUME_1_0);  /* = 0x0100 in 8.8 fixed = 1.0 (full volume) */
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   /* transformation matrix ... */
   BoxBufferWriteMatrix(pBuffer, (BMUXlib_File_MP4_P_Matrix *)&IdentityMatrix);
   /* predefined ... (older spec compatibility) */
   BoxBufferWriteU32(pBuffer, 0);
   BoxBufferWriteU32(pBuffer, 0);
   BoxBufferWriteU32(pBuffer, 0);
   BoxBufferWriteU32(pBuffer, 0);
   BoxBufferWriteU32(pBuffer, 0);
   BoxBufferWriteU32(pBuffer, 0);
   /* set next_track_id */
   BoxBufferWriteU32(pBuffer, hMP4Mux->uiLargestTrackID+1);    /* next_track_id */
   BoxBufferFinishBox(pBuffer);
   return true;
}

static bool CreateBoxTrak(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;

   BoxBufferStartBox(pBuffer);
   if (!CreateBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_TRAK, BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN))
      return false;        /* no room in buffer for the box */

   if (!CreateBoxTkhd(hMP4Mux))
      return false;

   /* if initial offset present, include edts box */
   if (0 != pTrack->uiInitialOffset90kHz)
      if (!CreateBoxEdts(hMP4Mux))
         return false;

   BoxBufferFinishBox(pBuffer);
   return true;
}

static bool CreateBoxTkhd(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   uint8_t uiBoxVersion = (pTrack->uiDuration90kHz > 0xFFFFFFFF)?
                              BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1:  /* use 64-bit fields if duration requires it */
                              BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0;

   uint32_t uiPLSize = (BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1 == uiBoxVersion)?
               BMUXLIB_FILE_MP4_P_PLSIZE_TKHD_1:BMUXLIB_FILE_MP4_P_PLSIZE_TKHD_0;
   uint32_t uiFlags = BMUXLIB_FILE_MP4_P_TKHD_FLAG_ENABLED | BMUXLIB_FILE_MP4_P_TKHD_FLAG_IN_MOVIE |
               BMUXLIB_FILE_MP4_P_TKHD_FLAG_IN_PREVIEW;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_TKHD, uiPLSize, uiBoxVersion, uiFlags))
      return false;        /* no room in buffer for the box */

   if (uiBoxVersion == BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1)
   {
      BoxBufferWriteU64(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* creation_time */
      BoxBufferWriteU64(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* modification time */
      BoxBufferWriteU32(pBuffer, pTrack->uiTrackID);                    /* track ID */
      BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
      /* NOTE: This duration includes all track edit list durations ... */
      BoxBufferWriteU64(pBuffer, pTrack->uiDuration90kHz);              /* duration (in 90kHz timescale) */
   }
   else
   {
      BoxBufferWriteU32(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* creation_time */
      BoxBufferWriteU32(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* modification time */
      BoxBufferWriteU32(pBuffer, pTrack->uiTrackID);                    /* track ID */
      BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
      /* NOTE: This duration includes all track edit list durations ... */
      BoxBufferWriteU32(pBuffer, pTrack->uiDuration90kHz);              /* duration (in 90kHz timescale) */
   }
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU16(pBuffer, 0);                                       /* layer */
   BoxBufferWriteU16(pBuffer, 0);                                       /* alternate group */
   if (BMUXlib_File_MP4_P_TrackType_eAudio == pTrack->eType)
      BoxBufferWriteU16(pBuffer, 0);                                    /* volume (audio) */
   else
      BoxBufferWriteU16(pBuffer, 0);                                    /* volume (non-audio) */
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteMatrix(pBuffer, (BMUXlib_File_MP4_P_Matrix *)&IdentityMatrix);
   BoxBufferWriteU32(pBuffer, pTrack->uiWidth << 16);                   /* track width (16.16 format) */
   BoxBufferWriteU32(pBuffer, pTrack->uiHeight << 16);                  /* track height (16.16 format) */
   return true;
}

/* Edit Box is used to handle offsets between streams
   For simplicity, an edit box is always added such that
   the edit list indicates a single segment of the entire
   presentation's duration.  If an initial offset is then
   present, an additional entry is added to the relevant
   track(s) to represent that offset
*/
static bool CreateBoxEdts(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   uint32_t uiPLSize = (pTrack->uiDuration90kHz > 0xFFFFFFFF)?
                              BMUXLIB_FILE_MP4_P_PLSIZE_EDTS_1:
                              BMUXLIB_FILE_MP4_P_PLSIZE_EDTS_0;

   if (!CreateBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_EDTS, uiPLSize))
      return false;        /* no room in buffer for the box */

   if (!CreateBoxElst(hMP4Mux))
      return false;

   return true;
}

static bool CreateBoxElst(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   uint8_t uiBoxVersion = (pTrack->uiDuration90kHz > 0xFFFFFFFF)?
                              BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1:  /* use a 64-bit fields if duration requires it */
                              BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0;
   uint32_t uiPLSize = (BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1 == uiBoxVersion)?
                     BMUXLIB_FILE_MP4_P_PLSIZE_ELST_1: BMUXLIB_FILE_MP4_P_PLSIZE_ELST_0;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_ELST, uiPLSize, uiBoxVersion, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU32(pBuffer, 2);                     /* entry-count - two fixed entries */

   /* starting offset entry (will indicate an offset of zero, if none exists)
      Assumes all tracks have a start time of zero, so this is simply the initial offset */
   /* BE CAREFUL! If media time is ever filled in with a value other than 0 or -1, then
      this needs conversion into MEDIA TIMESCALE!! */
   if (uiBoxVersion == BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1)
   {
      BoxBufferWriteU64(pBuffer, pTrack->uiInitialOffset90kHz);   /* starting offset duration */
      BoxBufferWriteU64(pBuffer, BMUXLIB_FILE_MP4_P_EMPTY_EDIT);  /* media time (empty edit) */
   }
   else
   {
      BoxBufferWriteU32(pBuffer, pTrack->uiInitialOffset90kHz);   /* starting offset duration */
      BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_EMPTY_EDIT);  /* media time (empty edit) */
   }
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RATE_1_0);       /* media rate - in 16.16 for normal usage */

   /* main presentation segment  */
   if (uiBoxVersion == BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1)
   {
      BoxBufferWriteU64(pBuffer, pTrack->uiDuration90kHz);        /* segment duration */
      BoxBufferWriteU64(pBuffer, 0);                              /* media time */
   }
   else
   {
      BoxBufferWriteU32(pBuffer, pTrack->uiDuration90kHz);        /* segment duration */
      BoxBufferWriteU32(pBuffer, 0);                              /* media time */
   }
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RATE_1_0);    /* media rate - in 16.16 for normal usage */
   return true;
}

static bool CreateBoxMdia(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   BoxBufferStartBox(pBuffer);
   if (!CreateBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_MDIA, BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN))
      return false;        /* no room in buffer for the box */

   if (!CreateBoxMdhd(hMP4Mux))
      return false;
   if (!CreateBoxHdlr(hMP4Mux))
      return false;

   BoxBufferFinishBox(pBuffer);
   return true;
}

static bool CreateBoxMdhd(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   /* recalculate audio duration in the relevant timescale
      (NOTE: it is extremely unlikely this calc can overflow) ... */
   uint64_t uiMediaDuration = (BMUXlib_File_MP4_P_TrackType_eAudio == pTrack->eType)?
                           ((pTrack->uiDuration90kHz * pTrack->uiTimescale) / BMUXLIB_FILE_MP4_P_TIMESCALE_90KHZ):
                           pTrack->uiDuration90kHz;
   /* determine the box version required based on the range of the duration (in the relevant timescale) ... */
   uint8_t uiBoxVersion = (uiMediaDuration > 0xFFFFFFFF)?
                              BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1:  /* use 64-bit fields if duration requires it */
                              BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0;

   uint32_t uiPLSize = (BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1 == uiBoxVersion)?
               BMUXLIB_FILE_MP4_P_PLSIZE_MDHD_1:BMUXLIB_FILE_MP4_P_PLSIZE_MDHD_0;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_MDHD, uiPLSize, uiBoxVersion, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   if (BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_1 == uiBoxVersion)
   {
      BoxBufferWriteU64(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* creation_time */
      BoxBufferWriteU64(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* modification time */
      BoxBufferWriteU32(pBuffer, pTrack->uiTimescale);                  /* media timescale */
      BoxBufferWriteU64(pBuffer, uiMediaDuration);                      /* duration (in the above timescale) */
   }
   else
   {
      BoxBufferWriteU32(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* creation_time */
      BoxBufferWriteU32(pBuffer, hMP4Mux->uiCreateTimeUTC);             /* modification time */
      BoxBufferWriteU32(pBuffer, pTrack->uiTimescale);                  /* media timescale */
      BoxBufferWriteU32(pBuffer, uiMediaDuration);                      /* duration (in the above timescale) */
   }
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_LANGUAGE_CODE_UND);    /* packed ISO 639 language */
   BoxBufferWriteU16(pBuffer, 0);                                       /* pre-defined */
   return true;
}

static bool CreateBoxHdlr(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_HDLR, BMUXLIB_FILE_MP4_P_PLSIZE_HDLR,
            BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU32(pBuffer, 0);                                       /* pre-defined */
   BoxBufferWriteFourCC(pBuffer, TrackToHandlerTable[pTrack->eType]);
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU8(pBuffer, BMUXLIB_FILE_MP4_P_NULL_CHAR);             /* name - empty string */
   return true;
}

static bool CreateBoxMinf(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;

   BoxBufferStartBox(pBuffer);

   /* NOTE: Size of the contents of the minf box depends on which media header is used */
   if (!CreateBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_MINF, BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN))
      return false;        /* no room in buffer for the box */

   /* Create media header for the type of track being processed ... */
   switch (pTrack->eType)
   {
      case BMUXlib_File_MP4_P_TrackType_eVideo:
         if (!CreateBoxVmhd(hMP4Mux))
            return false;
         break;
      case BMUXlib_File_MP4_P_TrackType_eAudio:
         if (!CreateBoxSmhd(hMP4Mux))
            return false;
         break;
      case BMUXlib_File_MP4_P_TrackType_eHint:
         if (!CreateBoxHmhd(hMP4Mux))
            return false;
         break;
      case BMUXlib_File_MP4_P_TrackType_eODSM:
         if (!CreateBoxNmhd(hMP4Mux))
            return false;
         break;
      case BMUXlib_File_MP4_P_TrackType_eSDSM:
         if (!CreateBoxNmhd(hMP4Mux))
            return false;
         break;
      default:
         /* ERROR: Should not happen! */
         BDBG_ERR(("Bad Track Type (%d) detected", pTrack->eType));
         break;
   }
   if (!CreateBoxDinf(hMP4Mux))
      return false;

   BoxBufferFinishBox(pBuffer);
   return true;
}

static bool CreateBoxVmhd(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_VMHD, BMUXLIB_FILE_MP4_P_PLSIZE_VMHD,
            BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_VMHD_FLAG))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU16(pBuffer, 0);      /* graphics mode (copy) */
   BoxBufferWriteU16(pBuffer, 0);      /* opcolor[0] - red */
   BoxBufferWriteU16(pBuffer, 0);      /* opcolor[1] - green */
   BoxBufferWriteU16(pBuffer, 0);      /* opcolor[2] - blue */
   return true;
}

static bool CreateBoxSmhd(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_SMHD, BMUXLIB_FILE_MP4_P_PLSIZE_SMHD,
            BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU16(pBuffer, 0);      /* balance */
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   return true;
}

/*
   Currently, we do not support Hint Tracks (no data will be placed in the track)
*/
static bool CreateBoxHmhd(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_HMHD, BMUXLIB_FILE_MP4_P_PLSIZE_HMHD,
            BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU16(pBuffer, 0);      /* maxPDUSize */
   BoxBufferWriteU16(pBuffer, 0);      /* avgPDUSize */
   BoxBufferWriteU32(pBuffer, 0);      /* maxbitrate */
   BoxBufferWriteU32(pBuffer, 0);      /* avgbitrate */
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   return true;
}

/* Null Media Header is an empty version 0 fullbox with no flags */
static bool CreateBoxNmhd(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_NMHD, BMUXLIB_FILE_MP4_P_PLSIZE_EMPTY,
            BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */
   /* There is no payload in a NULL header */
   return true;
}

static bool CreateBoxDinf(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   if (!CreateBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_DINF, BMUXLIB_FILE_MP4_P_PLSIZE_DINF))
      return false;        /* no room in buffer for the box */

   if (!CreateBoxDref(hMP4Mux))
      return false;
   return true;
}

static bool CreateBoxDref(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_DREF, BMUXLIB_FILE_MP4_P_PLSIZE_DREF,
            BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU32(pBuffer, 1);                  /* entry-count - one fixed entry */
   /* for now, this creates a single data entry ... */
   if (!CreateBoxDataEntryUrl(hMP4Mux))
      return false;
   return true;
}

/* for now, this data entry box s a NULL string, with flag set to indicate "media in file" */
static bool CreateBoxDataEntryUrl(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_URL, BMUXLIB_FILE_MP4_P_PLSIZE_URL,
            BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_URL_FLAG_MEDIA_IN_FILE))
      return false;        /* no room in buffer for the box */
   /* NOTE: since the "media in file" flag is set, this box must NOT contain a string - even an empty one! */
   return true;
}

/* Sample Table Box */
static bool CreateBoxStbl(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   BoxBufferStartBox(pBuffer);
   if (!CreateBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_STBL, BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN))
      return false;        /* no room in buffer for the box */
   /* the contents of this box vary based on the metadata collected, so these boxes are added
      on-the fly - this box would typically contain: stsd, stts, ctts, stsz, stsc, stco and stss */
   BoxBufferFinishBox(pBuffer);
   return true;
}

/* Sample Description Box

   NOTE: we do not (currently) support multiple sample descriptions.  This woule be required
   if the sequence information for a given set of samples changes such that a new sample
   description is required (e.g. width, height, frame rate, protocol, profile, level etc change)
*/
static bool CreateBoxStsd(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;

   BoxBufferStartBox(pBuffer);
   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_STSD,
         BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN + sizeof(uint32_t), /* account for entry-count field */
         BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU32(pBuffer, 1);       /* entry-count - one fixed entry */
   switch (pTrack->eType)
   {
      case BMUXlib_File_MP4_P_TrackType_eVideo:
         /* the format and content of this box depends on coding type ... */
         if (!CreateBoxVideoSampleEntry(hMP4Mux))
            return false;
         break;

      case BMUXlib_File_MP4_P_TrackType_eAudio:
         /* the format and content of this box depends on coding type ... */
         if (!CreateBoxAudioSampleEntry(hMP4Mux))
            return false;
         break;

      case BMUXlib_File_MP4_P_TrackType_eODSM:
      case BMUXlib_File_MP4_P_TrackType_eSDSM:
         if (!CreateBoxMpeg4SampleEntry(hMP4Mux))
            return false;
         break;

      /* For now we do not support the other track types ... */
      case BMUXlib_File_MP4_P_TrackType_eHint:
      default:
         BDBG_ERR(("Unsupported Track Type: %d", pTrack->eType));
         break;
   }

   BoxBufferFinishBox(pBuffer);
   return true;
}

/*
   Video Sample Entry *extends* the SampleEntry box, depending on the coding
   type (for example, AVC will add an avcC box)

   The "format" to use for this box depends on the type of stream being processed\
   (for example, AVC uses the "avc1" format code)
*/
static bool CreateBoxVideoSampleEntry(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   uint32_t uiPLSize;

   /* lookup box payload size based on coding type ... */
   switch (pTrack->eCoding)
   {
      case BMUXlib_File_MP4_P_CodingType_eAVC:
         /* PL Size for the AVC1 box is the size of the AVCC box it contains (including any header) */
         uiPLSize = BMUXLIB_FILE_MP4_P_BOX_HEADER_SIZE + BMUXLIB_FILE_MP4_P_PLSIZE_AVCC
            + hMP4Mux->uiAVCSPSLength + hMP4Mux->uiAVCPPSLength;
         break;

      case BMUXlib_File_MP4_P_CodingType_eMpeg4Video:
         uiPLSize = BMUXLIB_FILE_MP4_P_BOX_HEADER_SIZE + BMUXLIB_FILE_MP4_P_PLSIZE_ESDS;
         break;

      default:
         /* all other coding types do not add content */
         uiPLSize = 0;
         break;
   }

   /* the coding format to use for this box is determined from the coding type ... */
   if (!CreateBoxSampleEntry(hMP4Mux, BMUXLIB_FILE_MP4_P_PLSIZE_VIDEO_SAMPLE_ENTRY + uiPLSize))
      return false;        /* no room in buffer for the box */

   /* visual sample entry specific ... */
   BoxBufferWriteU16(pBuffer, 0);                           /* pre-defined */
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU32(pBuffer, 0);                           /* pre-defined ... */
   BoxBufferWriteU32(pBuffer, 0);
   BoxBufferWriteU32(pBuffer, 0);
   BoxBufferWriteU16(pBuffer, pTrack->uiWidth);             /* width */
   BoxBufferWriteU16(pBuffer, pTrack->uiHeight);            /* height */
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_VIDEO_RESOLUTION);  /* horiz resolution */
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_VIDEO_RESOLUTION);  /* vert resolution */
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_VIDEO_FRAME_COUNT); /* frame count per sample */
   BoxBufferFill(pBuffer, BMUXLIB_FILE_MP4_P_NULL_CHAR, BMUXLIB_FILE_MP4_P_VIDEO_COMP_NAME_LEN);   /* compressor name */
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_VIDEO_DEPTH);       /* depth */
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_VIDEO_PREDEFINED);  /* pre-defined */

   /* put codec-specific extension here (depends on coding type) ... */
   switch (pTrack->eCoding)
   {
      case BMUXlib_File_MP4_P_CodingType_eAVC:
         /* AVC-specific configuration ... */
         if (!CreateBoxAvcC(hMP4Mux))
            return false;
         /* if bitrate information known, then create a btrt box */
         break;

      case BMUXlib_File_MP4_P_CodingType_eMpeg4Video:
         if (!CreateBoxEsds(hMP4Mux))
            return false;
         break;

      default:
         /* do nothing */
         break;
   }
   return true;
}

/*
   Audio Sample Entry *extends* the SampleEntry box, depending on the coding type
   (for example, MPEG 4 Audio will add an esds box)

   The "format" to use for this box depends on the type of stream being processed
   (for example, MPEG 4 Audio uses the "mp4a" format code)
*/
static bool CreateBoxAudioSampleEntry(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   uint32_t uiPLSize;

   /* lookup box payload size based on coding type ... */

   switch (pTrack->eCoding)
   {
      case BMUXlib_File_MP4_P_CodingType_eMpeg4Audio:
         uiPLSize = BMUXLIB_FILE_MP4_P_FULLBOX_HEADER_SIZE + BMUXLIB_FILE_MP4_P_PLSIZE_ESDS;
         break;

      default:
         /* all other coding types do not add content */
         uiPLSize = 0;
         break;
   }

   /* the coding format to use for this box is determined from the coding type ... */
   if (!CreateBoxSampleEntry(hMP4Mux, BMUXLIB_FILE_MP4_P_PLSIZE_AUDIO_SAMPLE_ENTRY + uiPLSize))
      return false;        /* no room in buffer for the box */

   /* audio sample entry specific ... */
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_AUDIO_CHANNEL_COUNT);  /* channel count */
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_AUDIO_SAMPLE_SIZE);    /* sample size */
   BoxBufferWriteU16(pBuffer, 0);                                       /* pre-defined */
   BoxBufferWriteU16(pBuffer, BMUXLIB_FILE_MP4_P_RESERVED);
   BoxBufferWriteU32(pBuffer, pTrack->uiSampleRate << 16);              /* sample rate - 16.16 format */

   /* put codec specific extensions here (depends on coding type) ... */
   switch (pTrack->eCoding)
   {
      case BMUXlib_File_MP4_P_CodingType_eMpeg4Audio:
         if (!CreateBoxEsds(hMP4Mux))
            return false;
         break;

      default:
         /* do nothing */
         break;
   }
   return true;
}

/*
   Mpeg 4 Sample Entry *extends* the SampleEntry box by adding an esds box

   The "format" used for these boxes is the "mp4s" format code

   NOTE: this sample entry type is ONLY used for mpeg 4 systems streams
*/
static bool CreateBoxMpeg4SampleEntry(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;

   /* the coding format to use for this box is determined from the coding type ... */
   if (!CreateBoxSampleEntry(hMP4Mux, BMUXLIB_FILE_MP4_P_BOX_HEADER_SIZE + BMUXLIB_FILE_MP4_P_PLSIZE_ESDS))
      return false;        /* no room in buffer for the box */

   if (!CreateBoxEsds(hMP4Mux))
      return false;

   return true;
}

/*
   Baseline Sample Entry is extended by other information, depending on the
   stream and coding type.

   The "format" to use for this box depends on the coding type
   (for example, MPEG 4 SDS or ODS use the "mp4s" format code)

   uiBoxSize must be the size of the entire contents of any fields or boxes that
   extend this box

   The "format" of the box is determined by the coding type
*/
static bool CreateBoxSampleEntry(BMUXlib_File_MP4_Handle hMP4Mux, uint32_t uiPLSize)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;

   if (!CreateBox(pBuffer, CodingToFormatTable[pTrack->eCoding], BMUXLIB_FILE_MP4_P_PLSIZE_SAMPLE_ENTRY + uiPLSize))
      return false;        /* no room in buffer for the box */

   BoxBufferFill(pBuffer, 0, 6);                            /* reserved */
   /* NOTE: we only support a single data reference, hence this is fixed to 1 */
   BoxBufferWriteU16(pBuffer, 1);                           /* data_reference_index */
   return true;
}

/* For now, we create an avcC box with the SPS and PPS embedded.
   Even though the size of the PPS and SPS is variable, this box is
   a fixed-size box since we know the size of the contents at the
   point this box is created.

   FIXME: This is temporary - for now, we cheat and store the SPS/PPS in memory, but theoretically
   we are not permitted to read directly from CDB memory. In the future, this would be set to contain
   no SPS/PPS and we would use a parameter track, or we would need to store the SPS/PPS in a temp file
   such that it can be read back to populate the avcC box.
*/
static bool CreateBoxAvcC(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   uint32_t i;
   uint32_t uiPLSize = BMUXLIB_FILE_MP4_P_PLSIZE_AVCC + hMP4Mux->uiAVCSPSLength + hMP4Mux->uiAVCPPSLength;

   if (!CreateBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_AVCC, uiPLSize))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU8(pBuffer, BMUXLIB_FILE_MP4_P_AVCC_CONFIG_VER);
   BoxBufferWriteU8(pBuffer, hMP4Mux->aAVCSPSData[1]);      /* Profile */
   BoxBufferWriteU8(pBuffer, hMP4Mux->aAVCSPSData[2]);      /* profile compatibility */
   BoxBufferWriteU8(pBuffer, hMP4Mux->aAVCSPSData[3]);      /* Level */
   BoxBufferWriteU8(pBuffer, BMUXLIB_FILE_MP4_P_AVCC_RESERVED_3BITS | BMUXLIB_FILE_MP4_P_AVCC_NALU_LENGTH_SIZE_MINUS_1);
   BoxBufferWriteU8(pBuffer, BMUXLIB_FILE_MP4_P_AVCC_RESERVED_3BITS | BMUXLIB_FILE_MP4_P_AVCC_NUM_SEQ_PARAM_SETS);  /* num seq param sets = 1 */
   BoxBufferWriteU16(pBuffer, hMP4Mux->uiAVCSPSLength);
   /* copy SPS into the box buffer */
   /* FIXME: Do we need a BoxBufferCopy() function ?? */
   for (i = 0; i < hMP4Mux->uiAVCSPSLength; i++)
      BoxBufferWriteU8(pBuffer, hMP4Mux->aAVCSPSData[i]);
   BoxBufferWriteU8(pBuffer, BMUXLIB_FILE_MP4_P_AVCC_NUM_PIC_PARAM_SETS);  /* num pic param sets = 1 */
   BoxBufferWriteU16(pBuffer, hMP4Mux->uiAVCPPSLength);
   /* copy PPS into the box buffer */
   for (i = 0; i < hMP4Mux->uiAVCPPSLength; i++)
      BoxBufferWriteU8(pBuffer, hMP4Mux->aAVCPPSData[i]);
   return true;
}

/*
   Create an MPEG 4 Elementary Stream Descriptor Box
*/
static bool CreateBoxEsds(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;    /* needed for the size calculation */

   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_ESDS, BMUXLIB_FILE_MP4_P_DESC_SIZE_ES,
            BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;

   /* create an ES Descriptor in here; contents will depend on the protocol ... */
   if (!CreateMpeg4DescES(hMP4Mux))
      return false;

   return true;
}

/* Decoding Time To Sample table */
static bool CreateBoxStts(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   BMUXlib_File_MP4_P_MetadataInterface *pMetadata = &pTrack->aMetadata[BMUXlib_File_MP4_P_MetadataType_eStts];

   BoxBufferStartBox(pBuffer);
   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_STTS,
         BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN + sizeof(uint32_t), /* account for entry-count field */
         BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU32(pBuffer, pMetadata->uiEntryCount);        /* entry-count */
   /* the rest of this box is filled in by a special state to process stts metadata from storage
      (since its necessary to reload it from storage, which can block) */
   BoxBufferFinishBox(pBuffer);
   return true;
}

/* Composition Time To Sample table

   Note: This box is only required if CTS differs from DTS for any sample in the track
*/
static bool CreateBoxCtts(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   BMUXlib_File_MP4_P_MetadataInterface *pMetadata = &pTrack->aMetadata[BMUXlib_File_MP4_P_MetadataType_eCtts];

   BoxBufferStartBox(pBuffer);
   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_CTTS,
         BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN + sizeof(uint32_t), /* account for entry-count field */
         BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU32(pBuffer, pMetadata->uiEntryCount);        /* entry-count */
   /* the rest of this box is filled in by a special state to process ctts metadata from storage
      (since its necessary to reload it from storage, which can block) */
   BoxBufferFinishBox(pBuffer);
   return true;
}

/*
   Sample to Chunk conversion table

   NOTE: the current implementation does not create chunks - it assumes one sample per chunk, so this table
         is fixed.  Improvement can be made to coalesce adjacent samples from the same track into chunks,
         or (for video) create a gop per chunk structure if necessary
*/
static bool CreateBoxStsc(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   BoxBufferStartBox(pBuffer);
   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_STSC, BMUXLIB_FILE_MP4_P_PLSIZE_STSC,
            BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU32(pBuffer, 1);         /* entry-count - one fixed entry */
   BoxBufferWriteU32(pBuffer, 1);         /* first-chunk - always the first sample */
   BoxBufferWriteU32(pBuffer, 1);         /* samples per chunk - always one sample per chunk */
   BoxBufferWriteU32(pBuffer, 1);         /* samples description index - always the first */
   BoxBufferFinishBox(pBuffer);
   return true;
}

static bool CreateBoxStsz(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   BMUXlib_File_MP4_P_MetadataInterface *pMetadata = &pTrack->aMetadata[BMUXlib_File_MP4_P_MetadataType_eStsz];

   BoxBufferStartBox(pBuffer);
   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_STSZ, BMUXLIB_FILE_MP4_P_PLSIZE_STSZ,
            BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_SAMPLE_SIZE_ALL_DIFF);    /* sample size - all samples different */
   BoxBufferWriteU32(pBuffer, pMetadata->uiEntryCount);  /* sample count */
   /* the rest of the box (the array of sample sizes) is filled in by processing the stored metadata */
   BoxBufferFinishBox(pBuffer);
   return true;
}

/* Chunk offset table

   Since this implementation only creates one sample per chunk, this table is actually the
   sample offset table, and has one entry per sample

   NOTE: if the final size of the mdat exceeds 2^32, then 64-bit offsets are required, and
   a co64 box is required instead.  A co64 box is exactly the same except the offset table
   consists of 64-bit entries instead of 32-bit entries.
   The metadata used to create these boxes is stored in 64-bit format, and is converted as
   needed for the 32-bit version.
*/
static bool CreateBoxStco(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   BMUXlib_File_MP4_P_MetadataInterface *pMetadata = &pTrack->aMetadata[BMUXlib_File_MP4_P_MetadataType_eStco];
   uint32_t uiBoxType = (hMP4Mux->bCo64Required)?BMUXLIB_FILE_MP4_P_BOX_TYPE_CO64:BMUXLIB_FILE_MP4_P_BOX_TYPE_STCO;

   BoxBufferStartBox(pBuffer);
   if (!CreateFullBox(pBuffer, uiBoxType,
         BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN + sizeof(uint32_t), /* account for entry-count field */
         BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU32(pBuffer, pMetadata->uiEntryCount);        /* entry-count */
   /* the rest of this box (the offset table) is filled in by a special state to process
      metadata from storage (since its necessary to reload it from storage, which can block) */
   BoxBufferFinishBox(pBuffer);
   return true;
}

/*
   Sync Sample Table

   Each entry gives the sample number of samples that are Random-Access Points
   (Note: sample numbers start from 1).

   NOTE: This box is not required if ALL samples are RAP. This occurs for audio
   tracks, or some video streams (I-Frame only, for example)
*/
static bool CreateBoxStss(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;
   BMUXlib_File_MP4_P_MetadataInterface *pMetadata = &pTrack->aMetadata[BMUXlib_File_MP4_P_MetadataType_eStss];

   BoxBufferStartBox(pBuffer);
   if (!CreateFullBox(pBuffer, BMUXLIB_FILE_MP4_P_BOX_TYPE_STSS,
         BMUXLIB_FILE_MP4_P_PLSIZE_UNKNOWN + sizeof(uint32_t), /* account for entry-count field */
         BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_0, BMUXLIB_FILE_MP4_P_FULLBOX_FLAGS_NONE))
      return false;        /* no room in buffer for the box */

   BoxBufferWriteU32(pBuffer, pMetadata->uiEntryCount);        /* entry-count */
   /* the rest of this box (the Sample Number table) is filled in by a special state to process
      metadata from storage (since its necessary to reload it from storage, which can block) */
   BoxBufferFinishBox(pBuffer);
   return true;
}

/*
   Create a "fullbox" - this box has additional flags and version fields

   NOTE: uiBoxSize is the size of the data to be *contained* within this box
   - does not include the box headers
*/
static bool CreateFullBox(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, BMUXlib_File_MP4_FourCC uiBoxType, uint32_t uiBoxSize,
                  uint8_t uiVersion, uint32_t uiFlags)
{
   if (!CreateBox(pBuffer, uiBoxType, uiBoxSize + BMUXLIB_FILE_MP4_P_FULLBOX_VERSION_FLAGS_SIZE))
      return false;           /* no room in buffer for the box */
   /* write combined Version and Flags to box */
   BoxBufferWriteU32(pBuffer, (uiFlags | (uiVersion << 24)));
   return true;
}

/*
   Create a box in the box buffer

   BoxPLSize is the size of the data to be written into the box (NOT including the box type or size field)

   *Do not use this for creating large-size boxes: use CreateLargeBox() instead*

   NOTE: This code does not support a truly unbounded box (when in the MP4 file).  All boxes have their size field updated
   via an external mechanism once the final size is known (typically, this would be done via the storage interface
   by tracking bytes written and doing seek/write)

   For an unbounded box, when the box is written the size would contain the size appropriate for the
   amount of data written (size would contain the headers)
   - this is then updated with the actual size later *if* data is actually written to the box

   Returns true if successful, else returns false (no space in buffer)
*/
static bool CreateBox(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, BMUXlib_File_MP4_FourCC uiBoxType, uint32_t uiBoxPLSize)
{
   uint32_t uiSize = uiBoxPLSize + BMUXLIB_FILE_MP4_P_BOX_HEADER_SIZE;

   BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER, ("Create Box %c%c%c%c:: Request for %d bytes", uiBoxType >> 24,
      (uiBoxType >> 16) & 0xFF, (uiBoxType >> 8) & 0xFF, uiBoxType & 0xFF, uiSize));
   if (BoxBufferGetSpace(pBuffer) < uiSize)
   {
      BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER,("Out of space"));
      return false;                                /* no space in buffer for the box */
   }
#if BDBG_DEBUG_BUILD
   pBuffer->uiTotalBytesRequested += uiSize;
#endif
   /* write the size field */
   BoxBufferWriteU32(pBuffer, uiSize);
   /* write the box type */
   BoxBufferWriteFourCC(pBuffer, uiBoxType);
   return true;
}

/*
   Create a box that uses the largesize variant (i.e. size field is 1, and a 64-bit largesize field is present

   NOTE: mdat is the only box likely to require this, and it is not a fullbox, so the equivalent
   CreateLargeFullbox is probably not required

   By its very nature, such a large box typically will not fit into the box buffer, and thus the size cannot be
   known at the point the box is created.  Thus, this automatically assumes the box is unbounded and will write
   a zero size value.  It is expected that this size be later updated via Storage operations (seek/write),
   based upon the actual known size of the data in the box.

   The PLSize field allows for some partial contents to be written, and thus the available for space for this
   partial content can be checked in advance.

   As with CreateBox(), the PL size passed in includes all the contents of the box, but does not include
   the headers of the box itself (size, type, largesize)

   The PLSize MUST be correct for the amount of data (partial or not) that is written before another box is added,
   or before an unbounded box is finished.
*/
static bool CreateLargeBox(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, BMUXlib_File_MP4_FourCC uiBoxType, uint32_t uiBoxPLSize)
{
   uint32_t uiSize = uiBoxPLSize + BMUXLIB_FILE_MP4_P_BOX_HEADER_SIZE + BMUXLIB_FILE_MP4_P_BOX_LARGESIZE_FIELD_SIZE;

   /* check for available space (available space does not include the extended "unwrap" space) */
   BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER, ("Create Large Box %c%c%c%c:: Request for %d bytes", uiBoxType >> 24,
      (uiBoxType >> 16) & 0xFF, (uiBoxType >> 8) & 0xFF, uiBoxType & 0xFF, uiSize));

   if (BoxBufferGetSpace(pBuffer) < uiSize)
   {
      BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER,("Out of space"));
      return false;                                /* no space in buffer for the box */
   }

#if BDBG_DEBUG_BUILD
   pBuffer->uiTotalBytesRequested += uiSize;
#endif
   /* write the size field */
   BoxBufferWriteU32(pBuffer, BMUXLIB_FILE_MP4_P_BOX_SIZE_LARGESIZE);
   /* write the box type */
   BoxBufferWriteFourCC(pBuffer, uiBoxType);
   /* write the largesize field - this field MUST be updated after box is completed */
   BoxBufferWriteU64(pBuffer, BMUXLIB_FILE_MP4_P_BOX_SIZE_UNBOUNDED);
   return true;
}

/*******************************
   MPEG 4 Descriptor Creation
********************************/
static bool CreateMpeg4DescES(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;    /* needed for the size calculation */
   uint32_t uiPLSize = BMUXLIB_FILE_MP4_P_DESC_PLSIZE_ESD + BMUXLIB_FILE_MP4_P_DESC_SIZE_DEC_CONF + BMUXLIB_FILE_MP4_P_DESC_SIZE_SL_CONF;

   if (!CreateMpeg4Desc(pBuffer, BMUXLIB_FILE_MP4_P_DESC_ES, uiPLSize))
      return false;

   BoxBufferWriteU16(pBuffer, pTrack->uiTrackID);              /* ES ID (lower 16 bits of track ID) */
   BoxBufferWriteU8(pBuffer, BMUXLIB_FILE_MP4_P_ESD_FLAGS);    /* stream dep, URL, OCR stream, stream prio */

   if (!CreateMpeg4DescDecConfig(hMP4Mux))
      return false;

   if (!CreateMpeg4DescSLConfig(hMP4Mux))
      return false;

   return true;
}

/*
   Decoder Specific Configuration Descriptor
   Size depends on the data to be written - supplied by the encoder
*/
static bool CreateMpeg4DescDecConfig(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;    /* needed for the size calculation */
   /* size of this descriptor is the size of the fixed fields + size of the decoder specific info */
   uint32_t uiPLSize = BMUXLIB_FILE_MP4_P_DESC_PLSIZE_DEC_CONFIG + BMUXLIB_FILE_MP4_P_DESC_SIZE_DEC_SPEC;
   uint32_t uiStreamType;

   if (!CreateMpeg4Desc(pBuffer, BMUXLIB_FILE_MP4_P_DESC_DEC_CONF, uiPLSize))
      return false;

   /* decoder configuration information ... */
   BoxBufferWriteU8(pBuffer, CodingToObjTypeTable[pTrack->eCoding]);  /* object type indication */
   /* combine stream type, upStream, reserved and buffer size ... */
   uiStreamType = TrackToStreamTypeTable[pTrack->eType] << 2 | BMUXLIB_FILE_MP4_P_DESC_DECCONF_UPSTREAM | BMUXLIB_FILE_MP4_P_DESC_DECCONF_RES_1BIT;
   BoxBufferWriteU32(pBuffer, pTrack->uiDecodeBufferSize | (uiStreamType << 24));   /* stream type, upstream and buffer size DB */
   BoxBufferWriteU32(pBuffer, pTrack->uiMaxBitrate);      /* max bitrate */
   /* FIXME: according to ISO/IEC-14496-1, if stream is VBR, this (avg) bitrate should be zero (PI doesn't indicate VBR though) */
   BoxBufferWriteU32(pBuffer, pTrack->uiAvgBitrate);      /* avg bitrate */

   if (!CreateMpeg4DescDecSpecInfo(hMP4Mux))
      return false;
   return true;
}

/*
   Create a Decoder Specific Info Descriptor
*/
static bool CreateMpeg4DescDecSpecInfo(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;
   BMUXlib_File_MP4_P_TrackInfo *pTrack = hMP4Mux->pCurrentTrack;    /* needed for the size calculation */
   uint8_t *pWrite;
   uint32_t i;

   if (!CreateMpeg4Desc(pBuffer, BMUXLIB_FILE_MP4_P_DESC_DEC_SPEC, pTrack->uiDecSpecInfoSize))
      return false;

   /* decoder specific info ... */
   /* FIXME: Would it be useful to have a BoxBufferCopy() (copies in two chunks without having
      to check for wrap after each byte) ?? */
   pWrite = (uint8_t *)pBuffer->pTempWrite;
   for (i = 0; i < pTrack->uiDecSpecInfoSize; i++)
   {
      *pWrite++ = pTrack->aDecSpecInfo[i];
      (*pBuffer->pSize)++;
      pWrite = BoxBufferWrapCheck(pBuffer, pWrite);
   }
   pBuffer->pTempWrite = pWrite;
   return true;
}

/*
   SL Config descriptor requires use of predefined = 2, and contains no other information
*/
static bool CreateMpeg4DescSLConfig(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_BoxBuffer *pBuffer = &hMP4Mux->stBoxBuffer;

   if (!CreateMpeg4Desc(pBuffer, BMUXLIB_FILE_MP4_P_DESC_SL_CONF, BMUXLIB_FILE_MP4_P_DESC_PLSIZE_SL_CONFIG))
      return false;
   BoxBufferWriteU8(pBuffer, BMUXLIB_FILE_MP4_P_SL_CONF_PRE_DEF);
   return true;
}

/*
   Create an MPEG 4 Descriptor header.  This will always use the extended 4-byte size for
   maximum flexibility. These descriptors have a maxClassSize of 2^28 - 1
*/
static bool CreateMpeg4Desc(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint8_t uiDescTag, uint32_t uiDescPLSize)
{
   uint32_t uiSize = uiDescPLSize + BMUXLIB_FILE_MP4_P_DESC_HEADER_SIZE;

   BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER, ("Create MP4 Desc %s:: Request for %d bytes", DebugDescriptorTagTable[uiDescTag], uiSize));

   /* check for available space (available space does not include the extended "unwrap" space) */
   if (BoxBufferGetSpace(pBuffer) < uiSize)
   {
      BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER,("Out of space"));
      return false;                                /* no space in buffer for this descriptor */
   }

#if BDBG_DEBUG_BUILD
   pBuffer->uiTotalBytesRequested += uiSize;
#endif

   BoxBufferWriteU8(pBuffer, uiDescTag);
   BoxBufferWriteDescSize(pBuffer, uiDescPLSize);
   return true;
}

/**********************
  Box Buffer Handling
***********************/
/* Note: All multi-byte writes to the box buffer are in big-endian order
   Box buffer writes are byte-wise to avoid alignment exceptions */

/*
   Obtain the available space in the box buffer.
   NOTE: The space is from the temporary write pointer incase boxes
         have been written but buffer not updated yet
*/
static uint32_t BoxBufferGetSpace(BMUXlib_File_MP4_P_BoxBuffer *pBuffer)
{
   int iBytesAvail = pBuffer->pRead - pBuffer->pTempWrite - 1;
   /* check for available space (available space does NOT include the extended "unwrap" space) */
   if (iBytesAvail < 0)
      iBytesAvail += pBuffer->uiBufSize;
   BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER, ("Space Available = %d (pWrite = %p, pRead = %p)", iBytesAvail, pBuffer->pTempWrite, pBuffer->pRead));
   return (uint32_t)iBytesAvail;
}

/*
   Reset the box tracking information that tracks where the box starts and
   how many bytes were written, and whether that box is split over the end of
   the buffer
*/
static void BoxBufferStartBox(BMUXlib_File_MP4_P_BoxBuffer *pBuffer)
{
   pBuffer->pSize = &pBuffer->uiPart1Size;
   pBuffer->uiPart1Size = pBuffer->uiPart2Size = 0;
   pBuffer->pBoxStart = pBuffer->pWrite;
   pBuffer->pTempWrite = pBuffer->pWrite;
#if BDBG_DEBUG_BUILD
   pBuffer->uiTotalBytesRequested = 0;
#endif
   BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER, ("Starting New Box @ %p", pBuffer->pBoxStart));
}

/*
   Write a 64-bit unsigned integer to the box buffer
*/
/* NOTE: 64-bit writes are rarely used */
static void BoxBufferWriteU64(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint64_t uiData)
{
   uint8_t *pWrite = pBuffer->pTempWrite;
   uint8_t *pRead = (uint8_t *)&uiData;
   int i;
   for (i = sizeof(uint64_t)-1; i >= 0; i--)
      *pWrite++ = *(pRead+i);
   *(pBuffer->pSize) += sizeof(uint64_t);          /* count bytes written to this block */
   pBuffer->pTempWrite = BoxBufferWrapCheck(pBuffer, pWrite);
}

/*
   Write a 32-bit unsigned integer to the box buffer
*/
static void BoxBufferWriteU32(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint32_t uiData)
{
   uint8_t *pWrite = pBuffer->pTempWrite;
   *pWrite++ = (uint8_t)(uiData >> 24);
   *pWrite++ = (uint8_t)(uiData >> 16);
   *pWrite++ = (uint8_t)(uiData >> 8);
   *pWrite++ = (uint8_t)uiData;
   *(pBuffer->pSize) += sizeof(uint32_t);          /* count bytes written to this block */
   pBuffer->pTempWrite = BoxBufferWrapCheck(pBuffer, pWrite);
}

/*
   Write a 16-bit unsigned integer to the box buffer
*/
static void BoxBufferWriteU16(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint16_t uiData)
{
   uint8_t *pWrite = pBuffer->pTempWrite;
   *pWrite++ = (uint8_t)(uiData >> 8);
   *pWrite++ = (uint8_t)uiData;
   *(pBuffer->pSize) += sizeof(uint16_t);          /* count bytes written to this block */
   pBuffer->pTempWrite = BoxBufferWrapCheck(pBuffer, pWrite);
}

/*
   Write an 8-bit unsigned integer to the box buffer
*/
static void BoxBufferWriteU8(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint8_t uiByte)
{
   uint8_t *pWrite = pBuffer->pTempWrite;
   *pWrite++ = uiByte;
   (*pBuffer->pSize)++;
   pBuffer->pTempWrite = BoxBufferWrapCheck(pBuffer, pWrite);
}

/*
   Write an MPEG 4 Descriptor Size field
   This inserts the "next byte" markers into the length field, and makes
   it into a big-endian value
   NOTE: Size does not include the size field, or the tag
*/
static void BoxBufferWriteDescSize(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint32_t uiSize)
{
   uint8_t *pWrite = pBuffer->pTempWrite;
   uint8_t uiByte;

   /* check requested size does not exceed the max allowed for this descriptor ... */
   if (uiSize > BMUXLIB_FILE_MP4_P_MAX_DESC_SIZE)
   {
      /* should not happen - but if so, clip to max */
      BDBG_ERR(("Descriptor size (%d) exceeds max allowed - clipping size (output file will likely be corrupt)", uiSize));
      uiSize = BMUXLIB_FILE_MP4_P_MAX_DESC_SIZE;
   }
   uiByte = (uiSize >> 21) & BMUXLIB_FILE_MP4_P_DESC_SIZE_MASK;
   *pWrite++ = uiByte | BMUXLIB_FILE_MP4_P_DESC_SIZE_EXT_BIT;;
   uiByte = (uiSize >> 14) & BMUXLIB_FILE_MP4_P_DESC_SIZE_MASK;
   *pWrite++ = uiByte | BMUXLIB_FILE_MP4_P_DESC_SIZE_EXT_BIT;
   uiByte = (uiSize >> 7) & BMUXLIB_FILE_MP4_P_DESC_SIZE_MASK;
   *pWrite++ = uiByte | BMUXLIB_FILE_MP4_P_DESC_SIZE_EXT_BIT;
   uiByte = uiSize & BMUXLIB_FILE_MP4_P_DESC_SIZE_MASK;
   *pWrite++ = uiByte;
   *(pBuffer->pSize) += sizeof(uint32_t);          /* count bytes written to this block */
   pBuffer->pTempWrite = BoxBufferWrapCheck(pBuffer, pWrite);
}

/*
   Write a transformation matrix to the box buffer in big-endian order
*/
static void BoxBufferWriteMatrix(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, BMUXlib_File_MP4_P_Matrix *pMatrix)
{
   /* write each 32-bit entry in the matrix ... */
   BoxBufferWriteU32(pBuffer, pMatrix->a);
   BoxBufferWriteU32(pBuffer, pMatrix->b);
   BoxBufferWriteU32(pBuffer, pMatrix->u);
   BoxBufferWriteU32(pBuffer, pMatrix->c);
   BoxBufferWriteU32(pBuffer, pMatrix->d);
   BoxBufferWriteU32(pBuffer, pMatrix->v);
   BoxBufferWriteU32(pBuffer, pMatrix->x);
   BoxBufferWriteU32(pBuffer, pMatrix->y);
   BoxBufferWriteU32(pBuffer, pMatrix->w);
}

/*
   Fill the box buffer with the specified quantity of the specified 8-bit unsigned int
   (typically used to zero-fill box fields)
*/
/* TODO: This could probably be made more efficient by checking for split in the region
   to be filled and filling in two blocks, eliminating the Wrap check per byte (however,
   this is currently only used in two places, so may not be worth the effort!) */
static void BoxBufferFill(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, uint8_t uiData, uint32_t uiCount)
{
   uint8_t *pWrite = (uint8_t *)pBuffer->pTempWrite;
   while (uiCount)
   {
      *pWrite++ = uiData;
      (*pBuffer->pSize)++;
      pWrite = BoxBufferWrapCheck(pBuffer, pWrite);
      uiCount--;
   }
   pBuffer->pTempWrite = pWrite;
}

/*
   Check for write pointer wrap, and if so reset write pointer and
   switch the size tracking - returns the updated pointer
*/
static void *BoxBufferWrapCheck(BMUXlib_File_MP4_P_BoxBuffer *pBuffer, void *pWrite)
{
   /* check for wrap and then adjust pointer as necessary */
   if ((uint8_t *)pWrite >= pBuffer->pEnd)
   {
      /* if we wrap, switch to keeping track of part 2 - caller can then determine where data went */
      pBuffer->pSize = &pBuffer->uiPart2Size;
      return pBuffer->pBase;
   }
   else
      return pWrite;
}

/*
   Finalizes any prior writes to the box buffer to make them permanent.
   Until this function is called any prior writes can be abandoned if
   the box buffer runs out of space between writes, allowing the state
   machine to come back and try again when more space is available
*/
static void BoxBufferFinishBox(BMUXlib_File_MP4_P_BoxBuffer *pBuffer)
{
   /* copy the temporary write to the actual write pointer to make
      the writes "official" */
#if BDBG_DEBUG_BUILD
   /* largest instantaneous usage would be difference between Write and TempWrite */
   int32_t iUsed = pBuffer->pTempWrite - pBuffer->pWrite;
   if (iUsed < 0) iUsed += pBuffer->uiBufSize;
   if ((uint32_t)iUsed > pBuffer->uiMaxInstUsage)
      pBuffer->uiMaxInstUsage = iUsed;
   /* if the current pointer difference (temp write to write) exceeds
      the number of bytes used since BoxBufferStartBox then we have a problem!
      (implies there is a size calculation error somewhere - i.e. it wrote more
       bytes than was requested) */
   BDBG_ASSERT((uint32_t)iUsed <= pBuffer->uiTotalBytesRequested);
#endif

   pBuffer->pWrite = pBuffer->pTempWrite;
   BDBG_MODULE_MSG(BMUX_MP4_BOXBUFFER, ("Finish Box @ %p", pBuffer->pWrite));

#if BDBG_DEBUG_BUILD
   if ((pBuffer->uiBufSize - 1 - BoxBufferGetSpace(pBuffer)) > pBuffer->uiMaxUsage)
      pBuffer->uiMaxUsage = pBuffer->uiBufSize - 1 - BoxBufferGetSpace(pBuffer);
#endif
}

/********************
    Offset Stack
********************/

/* pop current value off the top of stack
   returns true if the returned value is valid, false if stack empty */
static bool OffsetStackPop(BMUXlib_File_MP4_P_OffsetStack *pStack, uint64_t *puiOffset)
{
   if (0 == pStack->uiTopStack)
   {
      *puiOffset = 0;
      BDBG_ERR(("Attempt to pop empty Offset Stack"));
      return false;
   }
   *puiOffset = pStack->aData[--pStack->uiTopStack];
   BDBG_MODULE_MSG(BMUX_MP4_STACK, ("Popping %lld from Offset Stack @ %d", *puiOffset, pStack->uiTopStack));
   return true;
}

/* add new value to top of stack
   returns true if the value was successfully added, false if the stack is full */
static bool OffsetStackPush(BMUXlib_File_MP4_P_OffsetStack *pStack, uint64_t uiOffset)
{
   if (BMUXlib_File_MP4_P_BoxNesting_eMax == pStack->uiTopStack)
   {
      BDBG_ERR(("Offset Stack Full"));
      return false;
   }
   BDBG_MODULE_MSG(BMUX_MP4_STACK, ("Storing %lld to Offset Stack @ %d", uiOffset, pStack->uiTopStack));
   pStack->aData[pStack->uiTopStack++] = uiOffset;
   return true;
}

/* peek at value on top of stack (without removing it)
   returns true if value is valid, false if stack empty */
static bool OffsetStackPeek(BMUXlib_File_MP4_P_OffsetStack *pStack, uint64_t *puiOffset)
{
   if (0 == pStack->uiTopStack)
   {
      *puiOffset = 0;
      BDBG_ERR(("Peeking at empty Offset Stack"));
      return false;
   }
   *puiOffset = pStack->aData[pStack->uiTopStack-1];
   return true;
}

/* directly modify the value at the top of the stack with the specified value
   returns true if successful, else false if stack empty */
static bool OffsetStackModify(BMUXlib_File_MP4_P_OffsetStack *pStack, uint64_t uiOffset)
{
   if (0 == pStack->uiTopStack)
   {
      BDBG_ERR(("Modifying an empty Offset Stack"));
      return false;
   }
   pStack->aData[pStack->uiTopStack-1] = uiOffset;
   return true;
}

/*****************************************************************************
* EOF
******************************************************************************/
