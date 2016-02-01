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
 * Private definitions for File-based MP4 software mux
 *
 * [Revision History:]
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BMUXLIB_FILE_MP4_PRIV_H__
#define BMUXLIB_FILE_MP4_PRIV_H__

/* Includes */
#include "bmuxlib_file_mp4.h"
#include "bmuxlib_file_mp4_metadata.h"
#include "bmuxlib_file_mp4_boxes.h"
#include "bmuxlib_input.h"

#ifdef BMUXLIB_MP4_P_TEST_MODE
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************
*  D E F I N I T I O N S    *
****************************/

#if BDBG_DEBUG_BUILD
#define DebugSetDescriptorSource(pOutput, where) pOutput->pDesc = where
#else
#define DebugSetDescriptorSource(pOutput, where)
#endif

#if BDBG_DEBUG_BUILD
extern const char * const DebugBlockedCauseTable[];
#endif

#define InitOutputCallback(hMP4Mux, eCallback, pData) \
   hMP4Mux->aOutputCallbacks[eCallback].pCallbackData = pData

/* accessor macros to allow tests to manipulate mux state */
#define BMUXLIB_FILE_MP4_P_GET_MUX_STATE(handle)         ((handle)->stStatus.eState)
#define BMUXLIB_FILE_MP4_P_SET_MUX_STATE(handle, state)  ((handle)->stStatus.eState = (state))

/******************************************************
  Descriptor Processing Macros
  (x is assumed to be a pointer to an input descriptor)
*******************************************************/
#define BMUXLIB_FILE_MP4_P_VIDEO_DESC_GET_DU_TYPE(x)  \
         (BMUXLIB_INPUT_DESCRIPTOR_VIDEO_IS_DATA_UNIT_START(x)?BMUXLIB_INPUT_DESCRIPTOR_VIDEO_DATA_UNIT_TYPE(x):0)

/* sample end occurs if start of next frame found, or if an empty frame marker is found
   (this assumes that empty frame descriptors can ONLY appear between frames) */
#define BMUXLIB_FILE_MP4_P_IS_SAMPLE_END(x)  \
         (BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART(x) || BMUXLIB_INPUT_DESCRIPTOR_IS_EMPTYFRAME(x))

/******************************************************
  Metadata Descriptor Processing Macros
  (x is assumed to be a pointer to a metadata descriptor)
*******************************************************/
/* video metadata validity flags ...*/
#define BMUXLIB_FILE_MP4_P_VIDEO_METADATA_IS_DIMENSION_VALID(x)  \
         (0 != ((x)->uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_DIMENSION_CODED_VALID))

#define BMUXLIB_FILE_MP4_P_VIDEO_METADATA_IS_BITRATE_VALID(x)  \
         (0 != ((x)->uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID))

#define BMUXLIB_FILE_MP4_P_VIDEO_METADATA_IS_FRAMERATE_VALID(x)   \
         (0 != ((x)->uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_FRAMERATE_VALID))

/* audio metadata validity flags ...*/
#define BMUXLIB_FILE_MP4_P_AUDIO_METADATA_IS_BITRATE_VALID(x)  \
         (0 != ((x)->uiMetadataFlags & BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID))

#define BMUXLIB_FILE_MP4_P_AUDIO_METADATA_IS_SAMPLINGFREQ_VALID(x)  \
         (0 != ((x)->uiMetadataFlags & BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_SAMPLING_FREQUENCY_VALID))

#define BMUXLIB_FILE_MP4_P_AUDIO_METADATA_IS_PROTOCOLDATA_VALID(x)   \
         (0 != ((x)->uiMetadataFlags & BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_PROTOCOL_DATA_VALID))

/********************
  Generic constants
********************/

/* This represents the mid-point of a full 33-bit PTS or DTS range
   => this is used for qualification of whether a PTS/DTS value
   precedes or follows another PTS/DTS value (modulo 33 bits)
   If the difference between two PTS or DTS values exceeds this
   amount it assumes that they have "wrapped" */
#define BMUXLIB_FILE_MP4_P_MODULO_33BITS_MID_RANGE ((uint32_t)-1)

/* reserved space (in bytes) in box buffer for "spillover"
   (this allows unwrapping of long writes so that pointer wrap checking can
    be performed per-write instead of per-byte)
   NOTE: This value assumes the largest permitted write to the buffer is
         a 64-bit value */
#define BMUXLIB_FILE_MP4_P_BOX_BUFFER_RESERVED  (8)

/* output indexes
   if mdat temp storage used it will occupy entry 1 (otherwise this entry will be empty)
   metadata output interfaces follow after "mdat" ... */
#define BMUXLIB_FILE_MP4_P_OUTPUT_MAIN          0
#define BMUXLIB_FILE_MP4_P_OUTPUT_MDAT          1
#define BMUXLIB_FILE_MP4_P_METADATA_START       2

/* maximum mdat size that can be tolerated before a co64 box becomes necessary for
   offset storage.  Note, this assumes that the moov and other headers occupy no
   more than 5% of the total file - this is only likely to be untrue for small
   files, in which case it does not matter, since offsets will be << 32-bits */
#define BMUXLIB_FILE_MP4_P_MAX_MDAT_FOR_STCO    0xF3333333

/* determine if DUs are expected in the descriptors for this input
   Currently, this is only true for AVC and MPEG 4 Part 2 */
/* here, x is the coding type */
#define BMUXLIB_FILE_MP4_P_IS_DU_ENABLED(x)     ((BMUXlib_File_MP4_P_CodingType_eAVC == (x)) || (BMUXlib_File_MP4_P_CodingType_eMpeg4Video == (x)))

/* H.264 NALU information */
#define BMUXLIB_FILE_MP4_P_AVC_NALU_TYPE_SPS    7
#define BMUXLIB_FILE_MP4_P_AVC_NALU_TYPE_PPS    8

/* here, x is expected to be the data unit type byte */
#define BMUXLIB_FILE_MP4_P_DU_IS_NALU_PPS(x)    (((x) & 0x1F) == BMUXLIB_FILE_MP4_P_AVC_NALU_TYPE_PPS)
#define BMUXLIB_FILE_MP4_P_DU_IS_NALU_SPS(x)    (((x) & 0x1F) == BMUXLIB_FILE_MP4_P_AVC_NALU_TYPE_SPS)

#define BMUXLIB_FILE_MP4_P_DU_IS_AVC_PARAM(x)   (BMUXLIB_FILE_MP4_P_DU_IS_NALU_PPS(x) || BMUXLIB_FILE_MP4_P_DU_IS_NALU_SPS(x))

/* NALU start code is expected to be 0x0000001 */
#define BMUXLIB_FILE_MP4_P_NALU_START_CODE_SIZE 4

/* MPEG4 Part 2 (mp4v) information */
/* NOTE: Video Object can have any start code 0x00 - 0x1F */
#define BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VIDOBJ        0x00
/* NOTE: Video Object Layer can have any start code 0x20 - 0x2F */
#define BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VIDOBJL       0x20
#define BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VISSEQ        0xB0
#define BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_USERDATA      0xB2
#define BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VISOBJ        0xB5

#define BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VIDOBJ_MASK   0xE0

/* here, x is expected to be the data unit type byte */
#define BMUXLIB_FILE_MP4_P_DU_IS_MP4V_VIDOBJ(x)    (((x) & BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VIDOBJ_MASK) == BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VIDOBJ)
#define BMUXLIB_FILE_MP4_P_DU_IS_MP4V_VIDOBJL(x)   (((x) & BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VIDOBJ_MASK) == BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VIDOBJL)
#define BMUXLIB_FILE_MP4_P_DU_IS_MP4V_VISSEQ(x)    ((x) == BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VISSEQ)
#define BMUXLIB_FILE_MP4_P_DU_IS_MP4V_VISOBJ(x)    ((x) == BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_VISOBJ)
#define BMUXLIB_FILE_MP4_P_DU_IS_MP4V_USERDATA(x)  ((x) == BMUXLIB_FILE_MP4_P_MP4V_DU_TYPE_USERDATA)

#define BMUXLIB_FILE_MP4_P_DU_IS_MP4V_CONFIG(x)    (BMUXLIB_FILE_MP4_P_DU_IS_MP4V_VISOBJ(x) || BMUXLIB_FILE_MP4_P_DU_IS_MP4V_VISSEQ(x) \
                                                      || BMUXLIB_FILE_MP4_P_DU_IS_MP4V_VIDOBJ(x) || BMUXLIB_FILE_MP4_P_DU_IS_MP4V_VIDOBJL(x))

/* 90kHz units for video duration, PTS, DTS, etc */
#define BMUXLIB_FILE_MP4_P_TIMESCALE_90KHZ      90000

/* defines an unused entry in the size buffer */
#define BMUXLIB_FILE_MP4_P_SIZE_ENTRY_UNUSED    ((uint32_t)-1)


/* next execution interval (in milliseconds) */
#define BMUXLIB_FILE_MP4_P_EXEC_INTERVAL        50
/* execution interval when finalizing the file
   (can be as fast as possible since we have no real-time requirement to worry about) */
#define BMUXLIB_FILE_MP4_P_EXEC_INTERVAL_NONE   0

/* indicate an invalid/undefined DTS value
   NOTE: since DTS values are 33 bits this value can never
   occur in a normal DTS */
#define BMUXLIB_FILE_MP4_P_INVALID_DTS          ((uint64_t)-1)

/**************
   Signatures
***************/

#define BMUXLIB_FILE_MP4_P_SIGNATURE_CREATESETTINGS      0x4D503401
#define BMUXLIB_FILE_MP4_P_SIGNATURE_STARTSETTINGS       0x4D503402
#define BMUXLIB_FILE_MP4_P_SIGNATURE_FINISHSETTINGS      0x4D503403
#define BMUXLIB_FILE_MP4_P_SIGNATURE_CONTEXT             0x4D503404

/* Typically, we need one storage descriptor for each input descriptor processed, with a worst case addition of one descriptor required
   to flush each metadata cache entry at the same time.
   Currently, the instantaneous usage is 1 descriptor for each NALU (input descriptor) in the input sample + 1 descriptor for the size
   field of each NALU (for current AVC output, we process 5 NALUs, and skip 2 NALUs in the the I-frame, thus requiring 10+2 descriptors)
   However, in theory, the absolute minimum required is 2 descriptors (for a split box), since all input descriptor processing can
   halt after each descriptor.
   NOTE: Lowering this value can severely affect data thruput, and can therefore prevent real-time operation */
#define BMUXLIB_FILE_MP4_P_NUM_OUT_DESC_MAIN             100

/* In theory, Metadata should only ever require one descriptor (since it should be returned before the next descriptor is
   queued for the same output) */
#define BMUXLIB_FILE_MP4_P_NUM_OUT_DESC_METADATA        5

/* number of outputs that are NOT used for metadata:
   one for final output + one for mdat temp storage */
#define BMUXLIB_FILE_MP4_P_NUM_NON_METADATA_OUTPUTS      2

/* Maximum number of freelist entries for creating release queues for resources,
   and for the callback entries:
   Worst case is total number of output descriptors for the non-metadata outputs
   (metadata outputs ONLY store metadata, and do not require release queues or
   callback data entries - release queues are only used on the input
   and box buffer sources, and callback entries are only used for inputs and size
   entries) */
#define BMUXLIB_FILE_MP4_P_NUM_FREELIST_ENTRIES          (BMUXLIB_FILE_MP4_P_NUM_NON_METADATA_OUTPUTS * BMUXLIB_FILE_MP4_P_NUM_OUT_DESC_MAIN)

/*********************
  Default Quantities
**********************/

/* by default, put the moov up front */
#define BMUXLIB_FILE_MP4_P_DEFAULT_PDL_SUPPORT           true

/* by default, assume the player supports edit lists for initial offset adjustments */
#define BMUXLIB_FILE_MP4_P_DEFAULT_EDIT_LIST_SUPPORT     true

/* for now, this is set to 10 iterations, assuming that execution interval
   is 50ms, giving a stall limit of approx. 500ms
   (nexus example apps show audio start lags video by about 350ms in some cases) */
/* SW7425-1228: Set this to zero for now to give same behaviour as ASF Mux */
#define BMUXLIB_FILE_MP4_P_DEFAULT_STALL_LIMIT           0

/* by default, finish processing all data in preparation for subsequent Stop() issued by app. */
#define BMUXLIB_FILE_MP4_P_DEFAULT_FINISH_MODE           BMUXlib_FinishMode_ePrepareForStop

#define BMUXLIB_FILE_MP4_P_MAX_ACTIVE_INPUTS             (BMUXLIB_FILE_MP4_MAX_VIDEO_INPUTS + BMUXLIB_FILE_MP4_MAX_AUDIO_INPUTS)

/* track support for: video/audio track + H.264 parameter set track, ODS and SDS and 1 spare, as needed ...
   (more tracks would be required to support hint tracks for example)
   NOTE: if more than one video, more than one h.264 param track may be required  */
#define BMUXLIB_FILE_MP4_P_MAX_TRACKS                    (BMUXLIB_FILE_MP4_P_MAX_ACTIVE_INPUTS + 4)

/* number of output (storage) interfaces required:
   number of non-metadata outputs + one per metadata type per track (for flushing the metadata cache) */
#define BMUXLIB_FILE_MP4_P_MAX_ACTIVE_OUTPUTS            (BMUXLIB_FILE_MP4_P_NUM_NON_METADATA_OUTPUTS + (BMUXLIB_FILE_MP4_P_MAX_TRACKS * BMUXlib_File_MP4_P_MetadataType_eMax))

/***************************************************************
   NOTE: The following Minimum Values must be larger than the largest instantaneous usage of each data type (i.e. this is the largest
   amount used at one time).  Enable BMUX_MP4_USAGE debug output to see the peak and instantaneous usages.
   Typically, these values would be much larger to prevent unnecessary "blocking" of the mux due to lack
   of resources. Keep in mind that if these values are made too small, such that it continually blocks waiting for resources, then
   the mux may not be able to process data fast enough to keep up with real time ...
****************************************************************/

/* Typically, we need one storage descriptor for each input descriptor processed, with a worst case addition of one descriptor required
   to flush each metadata cache entry at the same time.
   Currently, the instantaneous usage is 1 descriptor for each NALU (input descriptor) in the input sample + 1 descriptor for the size
   field of each NALU (for current AVC output, we process 5 NALUs, and skip 2 NALUs in the I-frame, thus requiring 10+2 descriptors)
   However, in theory, the absolute minimum required is 2 descriptors (for a split box), since all input descriptor processing can
   halt after each descriptor.
   NOTE: Lowering this value can affect data thruput, and can therefore prevent real-time operation
*/
#define BMUXLIB_FILE_MP4_P_MIN_NUM_OUT_STORE_DESC        BMUXLIB_FILE_MP4_P_NUM_OUT_DESC_MAIN

/* currently, the largest instantanous usage of this is the stsd box, at about 180-200 bytes for MPEG 4 Video
   NOTE: if this is set too small, the finish stage will never complete
   (times-out: always blocks with out-of-space result on Box Buffer)
   NOTE: this value represents the usable space - the actual buffer size is:
     BMUXLIB_FILE_MP4_P_MIN_BOX_HEAP_SIZE + BMUXLIB_FILE_MP4_P_BOX_BUFFER_RESERVED + 1
*/
#define BMUXLIB_FILE_MP4_P_MIN_BOX_HEAP_SIZE             1000     /* in bytes - used for constructing boxes */

/* size entries used for box size updates or for H.264 NALU size entries, etc. These are 32 bit values.
   For size entries, the absolute minimum is one entry, but of course this would cause unnecessary "blocking" of the mux */
#define BMUXLIB_FILE_MP4_P_MIN_NUM_SIZE_ENTRIES          100

/* For metadata entries, the absolute minimum is obviously one entry, but of course this would cause unnecessary "thrashing" of
   storage as the cache is flushed after each entry! The more the better to prevent unnecessary storage I/O operations. */
#define BMUXLIB_FILE_MP4_P_MIN_NUM_META_ENTRIES_CACHED   (32*1024)   /* each entry is essentially a sample (a frame of video or audio) */

/* temporary buffering used for storing chunks of the mdat during relocation
   for progressive download compatibility.  The larger this quantity, the
   less overhead during the mdat relocation. */
#define BMUXLIB_FILE_MP4_P_MIN_RELOCATION_BUFFER_SIZE    (128*1024)  /* in bytes - can be any size */

/* maximum size of decoder-specific data that can be stored for a track
   (used in the Decoder Config Descriptor in the Elementary Stream Descriptor) */
#define BMUXLIB_FILE_MP4_P_MAX_DEC_SPEC_SIZE             100

/* default values to use for missing metadata ... */
#define BMUXLIB_FILE_MP4_P_DEFAULT_VIDEO_WIDTH           1280
#define BMUXLIB_FILE_MP4_P_DEFAULT_VIDEO_HEIGHT          720
#define BMUXLIB_FILE_MP4_P_DEFAULT_VIDEO_BUFSIZE         0x10000

#define BMUXLIB_FILE_MP4_P_DEFAULT_AUDIO_SAMPLE_RATE     48000
#define BMUXLIB_FILE_MP4_P_DEFAULT_AUDIO_BUFSIZE         0x2000

/****************************
*        T Y P E S          *
****************************/

/* causes for ProcessInput() and ProcessBoxes() to be blocked ...
   (Note: causes that are not "Input" are reported to application as "Output"
    since all resource deficiencies are caused by output "backup"
    - resources are freed when output is completed) */
typedef enum
{
   BMUXlib_File_MP4_P_BlockedCause_eNone,
   BMUXlib_File_MP4_P_BlockedCause_eInput,               /* no available input descriptors */
   BMUXlib_File_MP4_P_BlockedCause_eOutput,              /* no available output descriptors */
   BMUXlib_File_MP4_P_BlockedCause_eBoxBuffer,           /* no space in box buffer */
   BMUXlib_File_MP4_P_BlockedCause_eSizeEntry,           /* no available size entries */
   BMUXlib_File_MP4_P_BlockedCause_eOutWait,             /* waiting for output transaction to complete */
   BMUXlib_File_MP4_P_BlockedCause_eCallbackData         /* no callback data entries available */
} BMUXlib_File_MP4_P_BlockedCause;

/* the following associates output sources to the callback that is used to free the
   relevant resource */
typedef enum
{
   BMUXlib_File_MP4_P_OutputCallback_eBoxBuffer,          /* source is Context.stBoxBuffer (pSource not used) */
   BMUXlib_File_MP4_P_OutputCallback_eInput,              /* pSource is one of the active inputs */
   BMUXlib_File_MP4_P_OutputCallback_eMetadataCache,      /* pSource is one of the metadata caches (metadata.stCache) */
   BMUXlib_File_MP4_P_OutputCallback_eSizeBuffer,         /* source is Context.stSizes (pSource contains the entry address) */
   /* NOTE: Relocation buffer has no callback - nothing to "free" */
   BMUXlib_File_MP4_P_OutputCallback_eMax,
   /* **** NOTE: this must be the last in the list***  */
   BMUXlib_File_MP4_P_OutputCallback_eNone
} BMUXlib_File_MP4_P_OutputCallback;

/* input descriptor processing state ...
   These states "block" when there are no more input descriptors, no available output descriptors,
   or no room in the size buffer (if processing AVC NALUs) */
typedef enum
{
   BMUXlib_File_MP4_P_InputState_eUnknown,
   BMUXlib_File_MP4_P_InputState_eStartup,               /* initial state to synchronize to SOF and process initial metadata */
   BMUXlib_File_MP4_P_InputState_eFindNewSample,         /* locate the new sample to process (the input with the lowest DTS) */
   BMUXlib_File_MP4_P_InputState_eProcessDescriptors     /* feed the input sample (descriptors) to the mdat */
} BMUXlib_File_MP4_P_InputState;

/* states for processing video descriptors ...
   (this necessary since AVC NALU data requires special processing)
   NOTE: state 0 MUST be the "Sample Start" state */
typedef enum
{
   BMUXlib_File_MP4_P_VideoState_eSampleStart = 0,
   BMUXlib_File_MP4_P_VideoState_ePopulateOutDesc,
   BMUXlib_File_MP4_P_VideoState_eNextDescriptor,
   /* the following are h.264-specific states: */
   BMUXlib_File_MP4_P_VideoState_eNALUSkipStartCode,
   BMUXlib_File_MP4_P_VideoState_eNALUCreateSize,
   BMUXlib_File_MP4_P_VideoState_eNALUPopulateOutDescSize,
   BMUXlib_File_MP4_P_VideoState_eNALUSizeUpdate,
   BMUXlib_File_MP4_P_VideoState_eNALUPopulateOutDescUpdate,
   BMUXlib_File_MP4_P_VideoState_eNALUProcessParams,
   /* end h.264-specific states */
   /* the following are mpeg4 part 2 specific states ... */
   BMUXlib_File_MP4_P_VideoState_eMP4VProcessConfig,
   /* end mpeg4 part 2 speific states */
   BMUXlib_File_MP4_P_VideoState_eNextDU
} BMUXlib_File_MP4_P_VideoState;

/* states for processing audio descriptors ...
   NOTE: state 0 MUST be the "Sample Start" state */
typedef enum
{
   BMUXlib_File_MP4_P_AudioState_eSampleStart = 0,
   BMUXlib_File_MP4_P_AudioState_ePopulateOutDesc,
   BMUXlib_File_MP4_P_AudioState_eNextDescriptor
} BMUXlib_File_MP4_P_AudioState;

/* the track type is used to define the "handler", media header and SampleEntry
   derivative to use for this track */
typedef enum
{
   BMUXlib_File_MP4_P_TrackType_eUnknown,
   /* NOTE: H.264 parameter tracks are also of type "video" */
   BMUXlib_File_MP4_P_TrackType_eVideo,
   BMUXlib_File_MP4_P_TrackType_eAudio,
   BMUXlib_File_MP4_P_TrackType_eHint,
   BMUXlib_File_MP4_P_TrackType_eODSM,
   BMUXlib_File_MP4_P_TrackType_eSDSM
} BMUXlib_File_MP4_P_TrackType;

/* the type of decoder to be used to process the content from the track */
typedef enum
{
   BMUXlib_File_MP4_P_CodingType_eUnknown,
   BMUXlib_File_MP4_P_CodingType_eAVC,                   /* ISO/IEC-14496-10 - "H.264/AVC" */
   BMUXlib_File_MP4_P_CodingType_eMpeg4Audio,            /* ISO/IEC-14496-3 - "AAC" */
   BMUXlib_File_MP4_P_CodingType_eMpeg4Video,            /* ISO/IEC-14496-2 - "MPEG 4 Part 2" */
   BMUXlib_File_MP4_P_CodingType_eMpeg4Systems,
   BMUXlib_File_MP4_P_CodingType_eVC1,
   BMUXlib_File_MP4_P_CodingType_eAVCParams,
   BMUXlib_File_MP4_P_CodingType_eAC3
} BMUXlib_File_MP4_P_CodingType;

typedef enum
{
   BMUXlib_File_MP4_P_DescriptorStatus_eFree,
   BMUXlib_File_MP4_P_DescriptorStatus_eWaiting,         /* waiting to be queued to the storage interface */
   BMUXlib_File_MP4_P_DescriptorStatus_eInUse            /* storage interface is using it and data is associated with it */
} BMUXlib_File_MP4_P_DescriptorStatus;

/* storage for size values that need to be independently sent to storage
   (for example for NALU sizes, or box size updates, etc)
   Note: this data is not necessarily released in order, so this may be sparse
   An unused entry is indicated by stored size value of zero
   Sizes stored here are stored in big-endian order! */
typedef struct
{
   uint32_t *pData;
   uint32_t uiNumEntries;
   uint32_t *pCurrentEntry;                              /* the current entry being used */
#if BDBG_DEBUG_BUILD
   uint32_t uiUsageCount;
   uint32_t uiMaxUsage;
   uint32_t uiMaxSearchDepth;
#endif
} BMUXlib_File_MP4_P_SizeStore;

/* release queue entry */
typedef struct BMUXlib_File_MP4_P_ReleaseQEntry
{
   uint32_t uiSequenceID;                                /* sequence number for this data (used for reordering for freeing) */
   uint8_t *pSourceData;                                 /* the data to free */
   uint32_t uiSourceLength;
   struct BMUXlib_File_MP4_P_ReleaseQEntry *pNext;
} BMUXlib_File_MP4_P_ReleaseQEntry;

typedef struct
{
   BMUXlib_File_MP4_P_ReleaseQEntry *pHead;
   BMUXlib_File_MP4_P_ReleaseQEntry *pTail;
} BMUXlib_File_MP4_P_ReleaseQ;

/* Entry for providing callback data to output callbacks so that sequence information can be passed back
   to allow freeing of source resources in the correct order
   NOTE: when chaining these into a free list, the pData field is used as a "next" pointer */
typedef struct BMUXlib_File_MP4_P_OutputCallbackData
{
   BMUXlib_File_MP4_Handle hMux;
   uint32_t uiSequenceID;                                /* sequence information used for reordering source resources */
   void *pData;                                          /* private data for the callback (usually the source of the output) */
} BMUXlib_File_MP4_P_OutputCallbackData;

/* internal "stream" representation */
typedef struct
{
   BMUXlib_Input_Handle hInput;                          /* handle of the input source for this input */
   struct BMUXlib_File_MP4_P_TrackInfo *pTrack;          /* the main track corresponding to this input */

   uint32_t uiWaitingCount;                              /* count of descriptors already passed to output descriptor queue or in the release Q */
   uint32_t uiStallCount;                                /* count of how many times this input had no descriptors available */
   uint32_t uiDescSeqCount;                              /* count to keep track of the descriptor order (to ensure freed in order) */
   uint32_t uiReleaseSeqCount;                           /* current sequence value of the descriptors being released */

   uint64_t uiInitialDTS;                                /* first DTS seen for this input (used for determining start offsets) */
   bool bEOS;                                            /* end-of-stream detected on this input */
   bool bDataProcessed;                                  /* indicates if any input descriptors were processed to the output for this input */

   BMUXlib_File_MP4_P_ReleaseQ stReleaseQ;               /* release Q for this input - describes the list of descriptors to be freed */

#if BDBG_DEBUG_BUILD
   uint32_t uiIndex;                                     /* the index of this input (debug only - for human consumption) */
#endif
} BMUXlib_File_MP4_P_Input;

/* structure to define an output (goes to storage)
   used for all output including mdat temp, metadata, parameter sets, etc */
typedef struct BMUXlib_File_MP4_P_Output
{
   BMUXlib_StorageObjectInterface stInterface;           /* the storage interface this output utilizes */
   BMUXlib_Output_Handle hOutput;                        /* handle of the output interface for this output */
#if BDBG_DEBUG_BUILD
   char *pDesc;                                          /* description of the source of the output descriptor */
#endif
} BMUXlib_File_MP4_P_Output;

/* the cache is used to store metadata locally before it is written to storage (in one big chunk) - this is to try to prevent thrashing of
   the disk (if the storage is file based) */
typedef struct
{
   BMUXlib_File_MP4_P_Output *pOutput;                   /* the output (storage) interface for this metadata */
   BMUXlib_File_MP4_P_MetadataCache stCache;
   uint32_t uiEntryCount;                                /* the total number of entries written to metadata storage */
   uint32_t uiCurrentValue;                              /* current value and count for metadata that is stored as run-counts */
   uint32_t uiRunCount;                                  /* (i.e. Stts and Ctts) */
   bool bInitialReadComplete;                            /* indicates when the initial metadata has been read back from storage */
} BMUXlib_File_MP4_P_MetadataInterface;

/* NOTE: This structure (of one storage per type of metadata) is to improve efficiency of moov creation, such that the information for
   each box to be written is contained contiguously within each metadata storage, rather than forcing the creation of the moov to
   scan the metadata storage several times for each box created. This allows the box creation to simply copy the metadata as-is into
   the specified box using more efficient block copy rather than sample-by-sample copy.
   If required, this code could be modified to support a single metadata storage, or maybe both mechanisms
*/
typedef struct BMUXlib_File_MP4_P_TrackInfo
{
   uint64_t uiDuration90kHz;                             /* duration of this track in 90kHz timescale */
   uint64_t uiTotalBytes;                                /* total amount of data processed for this track (used for bitrate calc) */
   uint32_t uiTrackID;                                   /* NOTE: Mpeg4 compliant tracks only use the lower 16 bits */
   uint32_t uiSampleCount;                               /* count of samples processed for this track */
   uint32_t uiAvgBitrate;                                /* average bitrate calculated from total bytes and duration */
   uint32_t uiInitialOffset90kHz;                        /* initial timing offset of this track from lead track (track with the lowest DTS) */
   uint32_t uiMaxSampleSize;                             /* used for calculation of buffering for audio */
   /* values for adjustment of offsets when mdat relocated ...
      Note: these offsets only need to be 32-bits, since the moov is
      at the start of the file in this case, and we do not support moov > 4Gb */
   uint32_t uiStcoOffset;                                /* starting offset (in the output storage) of the stco for this track */
   uint32_t uiStcoSize;                                  /* size in bytes of the stco for this track */
   /* codec-provided metadata ... */
   uint32_t uiWidth;                                     /* video-only */
   uint32_t uiHeight;                                    /* video-only */
   uint32_t uiMaxBitrate;
   uint32_t uiDecodeBufferSize;
   uint32_t uiSampleRate;                                /* audio-only */
   uint32_t uiTimescale;                                 /* timescale of the media (90khz for video, sample-rate for audio) */

   uint32_t uiDecSpecInfoSize;                           /* opaque decoder-specific information to be placed in the DecoderConfigDescriptor */
   uint8_t aDecSpecInfo[BMUXLIB_FILE_MP4_P_MAX_DEC_SPEC_SIZE];
   bool bDecSpecInfoDone;                                 /* ignore future occurrences of DecSpecInfo, if present */
   /* the following is for validation of DecSpecInfo for unexpected changes */
   bool bDecSpecInfoValidated;
   uint32_t uiDecSpecInfoValidateLength;

   bool bCttsRequired;                                   /* CTTS box is required for this track (i.e. CTS and DTS differ) */
   bool bStssRequired;                                   /* this track requires sync-sample table */

   BMUXlib_File_MP4_P_CodingType eCoding;                /* defines the codec to be used for this track */
   BMUXlib_File_MP4_P_TrackType eType;
   BMUXlib_File_MP4_P_Input *pInput;                     /* if this is NULL, this track is created indirectly by the mux from the inputs
                                                            (e.g. parameter set track, ODS, etc)  */
   /* Track metadata - one entry per type of metadata stored during mdat creation ... */
   BMUXlib_File_MP4_P_MetadataInterface aMetadata[BMUXlib_File_MP4_P_MetadataType_eMax];

} BMUXlib_File_MP4_P_TrackInfo;

/* structure used to describe data units signalled in the input descriptors
   (used for H.264 and Mpeg4 Part2 (mp4v) video) */
typedef struct
{
   uint8_t                             uiType;           /* type of the current data unit being processed */
   uint32_t                            uiLength;         /* accumulated length of the current DU */
   uint32_t                            uiBytesToSkip;    /* number of start code bytes that must still be skipped in this DU */
   uint64_t                            uiSizeOffset;     /* offset of the size field for this DU */
} BMUXlib_File_MP4_P_DataUnitInfo;

/* current state information and information collected for processing the current input descriptors */
typedef struct
{
   union
   {
      BMUXlib_File_MP4_P_VideoState Video;
      BMUXlib_File_MP4_P_AudioState Audio;
   } eState;                                             /* current input processing state - depends on input type */

   BMUXlib_Input_Descriptor            stInDesc;         /* current input descriptor being processed */
   uint32_t                            uiBytesSkipped;   /* number of bytes skipped in the current input descriptor being processed */

   BMUXlib_File_MP4_P_DataUnitInfo     stDataUnit;       /* information for data unit processing if required (video only) */
   BMUXlib_File_MP4_P_Metadata         stMetadata;       /* metadata collected for the current sample being processed */
   uint64_t                            uiDTS;            /* the DTS of the current sample being processed (used for deltaDTS calcs) */
   uint32_t                            uiDTSDelta90kHz;  /* delta DTS in 90kHz units for calculation of duration */

   BMUXlib_File_MP4_P_Input            *pInput;          /* the current input source for the current sample being processed */

   bool                                bComplete;        /* indicates this sample is complete (start of next sample detected) */
} BMUXlib_File_MP4_P_CurrentSample;

/* Information that is provided at Create() time - this should be considered to be READ-ONLY data!! */
typedef struct
{
   uint32_t                      uiSignature;

   BMUXlib_File_MP4_P_ReleaseQEntry *pReleaseQFreeList;  /* list of nodes to use for constructing release queues */
   uint32_t                      uiReleaseQFreeCount;

   BMUXlib_File_MP4_P_OutputCallbackData *pOutputCBDataFreeList;   /* list of entries for output callback data */
   uint32_t                      uiOutputCBDataFreeCount;

   uint32_t                      uiOutDescCount;         /* number of output descriptors for main and mdata outputs */

   /* locations of cache buffers from which per-track caches are allocated ... */
   struct {
      void *pBuffer;
      size_t uiEntrySize;
   } aMetadataCacheBuffer[BMUXlib_File_MP4_P_MetadataType_eMax];

   uint32_t                      uiMetadataCacheEntryCount; /* number of entries (samples) in each cache (per track) */

   uint8_t                       *pBoxBuffer;            /* box buffer for creating boxes */
   uint32_t                      uiBoxBufferSize;        /* total size of the buffer (NOT the usable space - some is reserved) */

   uint32_t                      *pSizeBuffer;           /* buffer for storing sizes for box updates, or NALU sizes */
   uint32_t                      uiSizeBufferEntryCount;

   uint8_t                       *pRelocationBuffer;     /* buffer used for relocation of the mdat if "moov at start" required */
   uint32_t                      uiRelocationBufferSize;

   uint32_t                      uiMuxId;
} BMUXlib_File_MP4_P_CreateData;

/****************************
*    Context definition     *
****************************/

typedef struct BMUXlib_File_MP4_P_Context
{
   /* box processing ... */
   BMUXlib_File_MP4_P_BoxState   eBoxState;              /* used in ProcessBoxes() */
   BMUXlib_File_MP4_P_TrackInfo  *pCurrentTrack;         /* the current track being processed into a trak box */
   BMUXlib_File_MP4_P_BoxType    eCurrentBox;            /* the current box being processed */
   uint32_t                      uiBoxIndex;             /* the current box in the sequence */
   BMUXlib_File_MP4_P_MetadataInterface *pCurrentMetadata;     /* the current metadata being processed */
   bool                          bHeaderDone;            /* header successfully written to output */

   /* input processing ... */
   BMUXlib_File_MP4_P_InputState eInputState;            /* used in ProcessInputDescriptors() */
   BMUXlib_File_MP4_P_Output     *pMdatOutput;           /* pointer to the current output used for the mdat (either aActiveOutputs[0] or aActiveOutputs[1] */
   BMUXlib_File_MP4_P_CurrentSample stCurrentSample;     /* information about the current sample being processed from input descriptors */
   bool                          bMetadataFinalized;     /* final update of run-length coded metadata completed */

   BMUXlib_StorageSystemInterface stStorage;             /* storage system interface to use for allocation of temp "files" */

   /* NOTE: main output is always index 0 and the interface for this is supplied by the application upon Start()
            mdat temp storage is index 1 and is only used if moov is to be at the start of the file.
            The rest of the outputs are metadata storage for each track */
   BMUXlib_File_MP4_P_Output     aActiveOutputs[BMUXLIB_FILE_MP4_P_MAX_ACTIVE_OUTPUTS];
   BMUXlib_File_MP4_P_Input      aActiveInputs[BMUXLIB_FILE_MP4_P_MAX_ACTIVE_INPUTS];

   BMUXlib_File_MP4_P_Input      *pLeadInput;            /* the "lead" input (the input that has the lowest initial DTS value) */
   BMUXlib_InputGroup_Handle     hInputGroup;            /* the group of inputs to use for input data */

   BMUXlib_File_MP4_P_TrackInfo  aTracks[BMUXLIB_FILE_MP4_P_MAX_TRACKS];
   uint32_t                      uiNumTracks;            /* number of tracks in use in the Tracks array */
   uint32_t                      uiLargestTrackID;       /* required by mvhd box - must be larger than the highest Track ID in use */
   bool                          bCo64Required;          /* 64-bit offsets are required due to large mdat size */

   BMUXlib_Output_CompletedCallbackInfo aOutputCallbacks[BMUXlib_File_MP4_P_OutputCallback_eMax];
   BMUXlib_File_MP4_P_BoxBuffer stBoxBuffer;             /* temporary "heap" for box creation (this is simply byte buffer) */
   BMUXlib_File_MP4_P_ReleaseQEntry *pReleaseQFreeList;  /* Head of the release Q free list */
   BMUXlib_File_MP4_P_OutputCallbackData *pOutputCBDataFreeList;  /* Head of callback data free list */
#if BDBG_DEBUG_BUILD
   uint32_t                      uiReleaseQUsageCount;
   uint32_t                      uiReleaseQMaxUsage;
   uint32_t                      uiCallbackDataUsageCount;
   uint32_t                      uiCallbackDataMaxUsage;
   uint32_t                      uiPrevPctComplete;      /* used for % complete finalization (finish) debug */
#endif
   BMUXlib_File_MP4_P_ReleaseQ   stBoxBufferReleaseQ;    /* release Q for the box buffer - describes the list of locations to be freed */
   BMUXlib_File_MP4_P_OffsetStack stOffsetStack;         /* output offset stack for nested boxes and NALU size updates */
   BMUXlib_File_MP4_P_SizeStore  stSizes;                /* storage for size values to be written to output (either NALU sizes, or box sizes, etc) */
   BMUXlib_File_MP4_P_BoxStack   stBoxStack;             /* box nesting stack */
   BMUXlib_File_MP4_P_RelocationBuffer stRelocationBuffer;  /* buffer used when relocating the mdat */

   uint32_t                      uiExpectedDurationMs;   /* allows for duration of up to approx 49 days */
   /* NOTE: 32-bits is enough for create year up to 2040 ... */
   uint32_t                      uiCreateTimeUTC;        /* for insertion into mvhd/tkhd (same time used for create/modify for all tracks) */
   uint64_t                      uiPresentationDuration; /* actual duration of the presentation in 90kHz units */
   uint64_t                      uiMdatSize;             /* the overall size of the mdat (used for determining size/presence/contents of certain boxes) */
   bool                          bMoovAtStart;
   uint32_t                      uiEstimatedMoovSizeBytes;/* if this is zero, size unknown */
   uint32_t                      uiNewMdatOffset;        /* does not need to be 64-bits since this location is the end of the moov */
   uint32_t                      uiFinalizationStep;     /* keep track of where we are in the finalization process */
   uint32_t                      uiTotalFinalizationSteps;

   /* FIXME: This is Temporary! For now we store the AVC SPS/PPS to later create the avcC box */
   uint8_t                       aAVCSPSData[100];
   uint8_t                       *pAVCSPSData;
   uint32_t                      uiAVCSPSLength;
   uint8_t                       aAVCPPSData[30];
   uint8_t                       *pAVCPPSData;
   uint32_t                      uiAVCPPSLength;
   bool                          bAVCSPSDone;
   bool                          bAVCPPSDone;
   /* the following are for validation of the SPS/PPS to detect undesired changes */
   uint32_t                      uiAVCSPSValidateLength;
   uint32_t                      uiAVCPPSValidateLength;
   bool                          bAVCSPSValidateDone;
   bool                          bAVCPPSValidateDone;

   BMUXlib_DoMux_Status          stStatus;               /* status returned to the application (contains top-level mux state) */

#ifdef BMUXLIB_MP4_P_TEST_MODE
   FILE *fpConfig;
#endif

   /***** THIS MUST REMAIN AT THE END OF THIS STRUCTURE *****/
   /* (this structure is not cleared out when context is re-initialized) */
   BMUXlib_File_MP4_P_CreateData stCreate;               /* information provided by Create() - typically allocated memory */
   /*********************************************************/
} BMUXlib_File_MP4_P_Context;

/****************************
*    P R O T O T Y P E S    *
****************************/

void      BMUXlib_File_MP4_P_InitializeContext(BMUXlib_File_MP4_Handle hMP4Mux);
BERR_Code BMUXlib_File_MP4_P_Start(BMUXlib_File_MP4_Handle hMP4Mux);
void      BMUXlib_File_MP4_P_Stop(BMUXlib_File_MP4_Handle hMP4Mux);

BMUXlib_File_MP4_P_CodingType BMUXlib_File_MP4_P_GetVideoCodingType(BAVC_VideoCompressionStd eProtocol);
BMUXlib_File_MP4_P_CodingType BMUXlib_File_MP4_P_GetAudioCodingType(BAVC_AudioCompressionStd eProtocol);

BERR_Code BMUXlib_File_MP4_P_ProcessInputDescriptors(BMUXlib_File_MP4_Handle hMP4Mux);
BERR_Code BMUXlib_File_MP4_P_ProcessOutputDescriptorsWaiting(BMUXlib_File_MP4_Handle hMP4Mux);
BERR_Code BMUXlib_File_MP4_P_ProcessOutputDescriptorsCompleted(BMUXlib_File_MP4_Handle hMP4Mux);

bool      BMUXlib_File_MP4_P_IsInputProcessingDone(BMUXlib_File_MP4_Handle hMP4Mux);
bool      BMUXlib_File_MP4_P_IsOutputProcessingDone(BMUXlib_File_MP4_Handle hMP4Mux);

uint32_t *BMUXlib_File_MP4_P_FindFreeSizeEntry(BMUXlib_File_MP4_P_SizeStore *pSizes);
void      BMUXlib_File_MP4_P_OutputDescriptorAppend(BMUXlib_File_MP4_Handle hMP4Mux, BMUXlib_File_MP4_P_Output *pOutput, void *pAddress, BMMA_Block_Handle hBlock, size_t uiBlockOffset, uint32_t uiLength, BMUXlib_File_MP4_P_OutputCallback eCallback);
void      BMUXlib_File_MP4_P_OutputDescriptorUpdate(BMUXlib_File_MP4_Handle hMP4Mux, BMUXlib_File_MP4_P_Output *pOutput, void *pAddress, uint32_t uiLength, uint64_t uiOffset, BMUXlib_File_MP4_P_OutputCallback eCallback);
void      BMUXlib_File_MP4_P_OutputDescriptorRead(BMUXlib_File_MP4_Handle hMP4Mux, BMUXlib_File_MP4_P_Output *pOutput, void *pAddress, uint32_t uiLength, BMUXlib_File_MP4_P_OutputCallback eCallback);
BMUXlib_File_MP4_P_OutputCallbackData *BMUXlib_File_MP4_P_NewOutputCallbackData(BMUXlib_File_MP4_Handle hMP4Mux, void *pData, uint32_t uiSeqCount);
void      BMUXlib_File_MP4_P_FreeOutputCallbackData(BMUXlib_File_MP4_Handle hMP4Mux, BMUXlib_File_MP4_P_OutputCallbackData *pCallbackData);

void      BMUXlib_File_MP4_P_WriteU32BE(uint32_t *puiDest, uint32_t uiData);
uint32_t  BMUXlib_File_MP4_P_ReadU32BE(uint32_t *puiSource);
void      BMUXlib_File_MP4_P_WriteU64BE(uint64_t *puiDest, uint64_t uiData);
uint64_t  BMUXlib_File_MP4_P_ReadU64BE(uint64_t *puiSource);

#ifdef __cplusplus
}
#endif


#endif /* BMUXLIB_FILE_MP4_PRIV_H__ */

/*****************************************************************************
* EOF
******************************************************************************/
