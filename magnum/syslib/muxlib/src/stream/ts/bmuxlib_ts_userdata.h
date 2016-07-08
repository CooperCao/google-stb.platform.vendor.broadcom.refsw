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

#ifndef BMUXLIB_TS_USERDATA_H_
#define BMUXLIB_TS_USERDATA_H_

#ifdef __cplusplus
extern "C" {
#endif

/* enable the following to allow processing of streams that have badly
   encoded PTS values - testing purposes only */
/*#define TS_UD_BAD_PTS_WORKAROUND*/

/* enable this to always show dropped data warnings */
/*#define TS_UD_ALWAYS_SHOW_DROPPED_PACKETS*/

/* enable the following to workaround out-of-order DTS from encoder */
#define TS_UD_BAD_ENCODE_OUT_WORKAROUND

#ifdef BMUXLIB_TS_P_TEST_MODE
#include <stdio.h>
#endif

/*************************/
/*   Constants           */
/*************************/
/* scaling factor for converting PTS (90kHz) to ESCR (27MHz) */
#define BMUXLIB_TS_SCALE_PTS_TO_ESCR            300

/* 50ms window in 45kHz (PTS) units */
#define BMUXLIB_TS_USERDATA_DEFAULT_WINDOW      (50 * 45)

/* max number of segments a single userdata PES can be split into:
   (header prior to PTS, PTS, DTS(if present), data after PTS)
   Packets that do not contain a PES will use only one segment */
#define BMUXLIB_TS_USERDATA_MAX_SEGMENTS        4

/* Userdata requires approx 6 TS packets per video frame
   (max of 1128 bytes per frame for North American VBI, as per
    ANSI_SCTE 127, Section 8.1)
   or 8 TS packets per video frame (max of 1504 bytes per frame,
   for ETSI Teletext, as per ETSI EN 300 472, Section 5)
   Thus, we assume worst case requirement of 8 TS packets per
   video frame, per userdata input.
*/
#define BMUXLIB_TS_USERDATA_MAX_PKTS_PER_VID    8

/* This defines the selection window for choosing userdata that matches the source
   video frame.  Note that due to rate conversion, this allows for selection of
   up to 3 matching frames to allow for 60-24fps conversion
   (i.e. 3 source frames worth of data would need to be captured per output frame) */
/* FIXME: if we allow 15fps encode, we need to allow for a window of 4 frames */
#define BMUXLIB_TS_USERDATA_SELECTION_WIN_FRAMES   3

/* pre-transmission time - amount of time ahead of the PTS that the data is transmitted
   2 dPTS is maximum transmission to presentation delay suggested by ANSI SCTE 127
  (i.e. data should arrive no more than 2 frame-times ahead of the frame it belongs to) */
#define BMUXLIB_TS_USERDATA_PTT_FRAMES          2

/*
 * TS Packet constants
 */
#define BMUXLIB_TS_PACKET_SYNC                  0x47
#define BMUXLIB_TS_PAYLOAD_LENGTH               184
#define BMUXLIB_TS_HEADER_LENGTH                4
#define BMUXLIB_TS_PACKET_LENGTH                (BMUXLIB_TS_HEADER_LENGTH + BMUXLIB_TS_PAYLOAD_LENGTH)

/*
 * PES Packet constants
 */

/* A PTS or DTS entry in a PES header requires 5 bytes */
#define BMUXLIB_PES_PTS_LENGTH                  5

/* number of bytes in PES header up to and including the length */
#define BMUXLIB_PES_HEADER_LENGTH               6

/* length of PES optional fields flags and header_data_length */
#define BMUXLIB_PES_HEADER_FLAGS_LENGTH         3

/* header length up to and including DTS (if present)
   This is the minimum number of bytes we need to process the PES packet s*/
#define BMUXLIB_PES_MIN_HEADER_LENGTH           (BMUXLIB_PES_HEADER_LENGTH + BMUXLIB_PES_HEADER_FLAGS_LENGTH + (BMUXLIB_PES_PTS_LENGTH * 2))

/* ISO/IEC-13818-1 Private Stream 2 */
#define BMUXLIB_PES_TYPE_PSM                 0xBC
#define BMUXLIB_PES_TYPE_PADDING             0xBE
#define BMUXLIB_PES_TYPE_PRIVATE_2           0xBF
#define BMUXLIB_PES_TYPE_ECM                 0xF0
#define BMUXLIB_PES_TYPE_EMM                 0xF1
#define BMUXLIB_PES_TYPE_DSMCC               0xF2
#define BMUXLIB_PES_TYPE_H222_TYPE_E         0xF8
#define BMUXLIB_PES_TYPE_PSD                 0xFF

/*
 * Field test/access macros
 */
#define BMUXLIB_TS_GET_PID(ts)   \
            (((uint16_t)(ts[1] & 0x1F) << 8) | ts[2])

#define BMUXLIB_TS_GET_CC(ts)    \
            ((uint8_t)(ts[3] & 0xF))

#define BMUXLIB_TS_IS_PAYLOAD_START(ts) \
            (0 != ((ts)[1] & 0x40))

#define BMUXLIB_TS_IS_ADAPT_PRESENT(ts) \
            (0 != ((ts)[3] & 0x20))

#define BMUXLIB_TS_IS_PAYLOAD_PRESENT(ts) \
            (0 != ((ts)[3] & 0x10))

#define BMUXLIB_TS_GET_ADAPT_LEN(ts)   (ts)[4]

#define BMUXLIB_TS_IS_DISCONTINUITY(ts)   \
            (0 != ((ts)[5] & 0x80))

#define BMUXLIB_PES_IS_PTS_PRESENT(pes) \
            (0 != ((pes)[7] & 0x80))

#define BMUXLIB_PES_IS_DTS_PRESENT(pes) \
            (0 != ((pes)[7] & 0x40))

/* determine if the PES has the optional
   fields after the length (e.g. PTS/DTS etc)
   (i.e. this is not a private stream, padding
    stream, etc) */
#define BMUXLIB_PES_HAS_OPTIONAL_FIELDS(id) \
   (((id) != BMUXLIB_PES_TYPE_PSM) && ((id) != BMUXLIB_PES_TYPE_PADDING) && ((id) != BMUXLIB_PES_TYPE_PRIVATE_2)  \
     && ((id) != BMUXLIB_PES_TYPE_ECM) && ((id) != BMUXLIB_PES_TYPE_EMM) && ((id) != BMUXLIB_PES_TYPE_DSMCC)   \
     && ((id) != BMUXLIB_PES_TYPE_H222_TYPE_E) && ((id) != BMUXLIB_PES_TYPE_PSD))

#define BMUXLIB_PES_GET_STREAM_ID(pes) (pes)[3]

#define BMUXLIB_PES_GET_LENGTH(pes)  \
            (((uint16_t)((pes)[4]) << 8) | (pes)[5])

#define BMUXLIB_PES_GET_HDR_DATA_LEN(pes) (pes)[8]

#define BMUXLIB_PES_GET_PTS(pes)  \
            ((((uint64_t)((pes)[9] & 0x0e)) << 29) | \
            (((uint64_t)(pes)[10]) << 22) | \
            (((uint64_t)((pes)[11] & 0xFE)) << 14) | \
            (((uint64_t)(pes)[12]) << 7) | \
            (((uint64_t)((pes)[13] & 0xFE)) >> 1))

#define BMUXLIB_PES_GET_DTS(pes) \
            ((((uint64_t)((pes)[14] & 0x0e)) << 29) | \
            (((uint64_t)(pes)[15]) << 22) | \
            (((uint64_t)((pes)[16] & 0xFE)) << 14) | \
            (((uint64_t)(pes)[17]) << 7) | \
            (((uint64_t)((pes)[18] & 0xFE)) >> 1))

/* macro for writing a PTS or DTS field into a PTS entry */
#define BMUXLIB_TS_USERDATA_SET_PTS_DTS(buffer, pts)   \
            (buffer)[4] = (((pts) << 1) & 0xFE) | 0x01; \
            (buffer)[3] = (((pts) >> 7) & 0xFF); \
            (buffer)[2] = (((pts) >> 14) & 0xFE) | 0x01; \
            (buffer)[1] = (((pts) >> 22) & 0xFF); \
            (buffer)[0] = (((pts) >> 29) & 0x0E) | 0x21

/*************************/
/*   Definitions         */
/*************************/

/* specific to companion video - applies to all userdata inputs that refer to this companion video */
typedef struct BMUXlib_TS_P_UserdataVideoInfo
{
   uint16_t uiPID;                  /* PID of the companion video input */
   bool bIgnoreGOP;                 /* ignore GOPs with bad timing during startup */
   bool bFirstRAP;
   bool bStartup;                   /* true = skip initial blank frames and badly timed data */
   bool bPrevDTSValid;
   uint32_t uiTargetPTS45kHz;       /* target PTS in the userdata domain for the current companion video frame */
   uint32_t uiTimingAdjust45kHz;    /* timing adjustment to convert from source timing to encoder timing */
   uint32_t uiPrevDTS45kHz;
   uint32_t uiPreTransmission;      /* pre-transmission time of userdata relative to video data in 45kHz */
   uint32_t uiSelectionWindow;      /* the current selection window for determining which userdata PES to process with the current frame */
   uint32_t uiOrigPTS;              /* original video PTS for cross-reference; also serves as prev OPTS for change detection */
} BMUXlib_TS_P_UserdataVideoInfo;

typedef struct BMUXlib_TS_P_UserdataReleaseQEntry
{
   uint32_t uiSequenceID;              /* identifies the correct release order */
   size_t uiDescCount;                 /* number of sequential descriptors coalesced into a single transport descriptor */
   size_t uiDataLength;                /* number of bytes to consume */
   struct BMUXlib_TS_P_UserdataReleaseQEntry *pNext;
} BMUXlib_TS_P_UserdataReleaseQEntry;

typedef struct
{
   BMUXlib_TS_P_UserdataReleaseQEntry *pHead;
   BMUXlib_TS_P_UserdataReleaseQEntry *pTail;
   uint32_t uiSequenceID;              /* SW7425-3250: the current sequence number of the outgoing segment */
   uint32_t uiExpectedSeqID;           /* next expected sequence number to be freed */
} BMUXlib_TS_P_UserdataReleaseQ;

typedef struct
{
   uint32_t uiBytesProcessed;          /* num bytes processed in this TS packet - for locating PES header */
   bool bParsed;                       /* indicates this packet has already been parsed */
   bool bPayloadStart;                 /* PES payload starts in this packet */
   bool bAdaptationPresent;
   bool bPayloadPresent;
   bool bPTSPresent;
   bool bDTSPresent;
   uint32_t uiPTS45kHz;
   uint32_t uiDTS45kHz;
   uint16_t uiPID;
   uint8_t uiCC;
} BMUXlib_TS_P_UserdataPacketInfo;

/* userdata information specific to a particular userdata input */
typedef struct BMUXlib_TS_P_UserdataInfo
{
   bool bDisabled;                  /* indicates the input is disabled due to PID conflict */
   bool bFirstPESSeen;              /* if the first PES header has not been seen, we need to drop payloads */
   bool bDropPES;                   /* drop the current PES packet - flag is sticky until next PUSI seen; it will drop any packet with a payload until PUSI */
   bool bUnwrapInUse;               /* true if the unwrap buffer is being used - for error checking */
   bool bPIDValid;                  /* the PID for this input has been determined */
   bool bPrevCCValid;               /* continuity count value has been initialized */
   uint8_t *pCurrentPacket;         /* location of current TS Packet (either in unwrap buffer, or from userdata input) */
   uint8_t *pUnwrap;                /* location of storage for unwrapping a TS packet */
   uint32_t uiPendingBytes;         /* number of bytes pending for this userdata input */
   uint32_t uiIndex;                /* index of this userdata input */
   uint32_t uiSequenceCount;        /* monotonically increasing sequencing number for release Q */
   BMUXlib_TS_P_UserdataReleaseQ stReleaseQ;
   BMUXlib_TS_P_UserdataVideoInfo *pCompanionVideo;
   BMUXlib_TS_P_UserdataPacketInfo stPacketInfo;   /* info about the current packet being parsed */
   uint16_t uiPID;                  /* The PID detected corresponding to this input */
   uint8_t uiPrevCC;                /* the previous continuity count value seen for this input */
#ifndef TS_UD_ALWAYS_SHOW_DROPPED_PACKETS
   bool bPacketAccepted;            /* for debug: indicates PES has been processed, so warnings are not due to startup conditions */
#endif
#ifdef TS_UD_BAD_PTS_WORKAROUND
   /* FIXME: The following is a hack for working around badly encoded PTS - for testing only */
   uint32_t uiTimingOffset45kHz;    /* NOTE: this should be zero for properly encoded streams */
#endif
   BMMA_Block_Handle hBlock;        /* handle of the memory block the data for this input comes from */
   void *pBlockBase;                /* base virtual address of the input buffer for this input */
} BMUXlib_TS_P_UserdataInfo;

/* the user data (TS) packets to be queued to transport */
typedef struct
{
   bool bESCRValid;                    /* indicates whether this Packet has a valid ESCR or not
                                          (packets belonging to PES without PTS, or packets without
                                          a payload, have no ESCR) */
   uint32_t uiESCR;                    /* the scheduling time for this Packet */
   uint32_t uiNumSegments;             /* Total number of data segments that make up this PES */
   uint32_t uiSequenceCount;           /* SW7425-3250: Sequence count used to ensure packets are released in order */
   struct stSegment
   {
      uint8_t *pData;                  /* NOTE: This pointer is only used for PTS or unwrap (local storage) */
      size_t uiOffset;                 /* NOTE: this offset is currently only used for userdata input data */
      uint32_t uiLength;
      BMUXlib_TS_P_DataType eDataType; /* the type of the data pointed to by pData (may be either userdata, local storage or PTS entry) */
                                       /* Note: Source is always "Userdata" to ensure bytes get freed from userdata input */
      uint64_t uiTimestamp;
   } aSegments[BMUXLIB_TS_USERDATA_MAX_SEGMENTS];
} BMUXlib_TS_P_UserdataPending;

/* PTS entry for storing the modified PTS value for output to transport
   (since we are not permitted to modify the incoming userdata in-place) */
typedef struct
{
   uint8_t aPTS[BMUXLIB_PES_PTS_LENGTH];
} BMUXlib_TS_P_UserdataPTSEntry;

/* general status that applies to all userdata */
typedef struct BMUXlib_TS_P_UserdataStatus
{
   uint32_t uiNumInputs;
   uint32_t uiCurrentScheduledInput;   /* specifies the current userdata input that is due to be scheduled */
   uint32_t uiCurrentVideoPID;
#ifdef BMUXLIB_TS_P_TEST_MODE
   FILE *hUserdataCSV;
   bool bUserdataCSVOpened;
#endif
} BMUXlib_TS_P_UserdataStatus;

/* information about the incoming data for this userdata input (obtained from GetUserdataBuffer API) */
typedef struct
{
   size_t uiOffset0, uiOffset1;     /* offset to the data in the input (relative to the virtual point from BMMA_Lock()) */
   size_t uiBytes0, uiBytes1, uiBytesTotal;
} BMUXlib_TS_P_UserdataInput;

/*************************/
/*  Private Userdata API */
/*************************/

BERR_Code   BMUXlib_TS_P_UserdataInit(BMUXlib_TS_Handle hMuxTS);

BERR_Code   BMUXlib_TS_P_Userdata_ProcessInputs(BMUXlib_TS_Handle hMuxTS);

void        BMUXlib_TS_P_Userdata_AddToReleaseQ(BMUXlib_TS_Handle hMuxTS, uint32_t uiUserdataIndex, uint32_t uiLength, uint32_t uiSequenceCount, uint32_t uiDescCount);

BERR_Code   BMUXlib_TS_P_Userdata_ProcessReleaseQueues(BMUXlib_TS_Handle hMuxTS);

void        BMUXlib_TS_P_Userdata_SchedulePackets(BMUXlib_TS_Handle hMuxTS);

bool        BMUXlib_TS_P_Userdata_FindTargetPTS(BMUXlib_TS_Handle hMuxTS, BMUXlib_TS_P_InputMetaData *pInput, BMUXlib_Input_Descriptor *pstDescriptor);

bool        BMUXLIB_P_Userdata_CheckResources(BMUXlib_TS_Handle hMuxTS);

#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_TS_USERDATA_H_ */
