/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#ifndef BMUXLIB_TS_PRIV_H_
#define BMUXLIB_TS_PRIV_H_

#include "bmuxlib_ts_consts.h"
#include "bmuxlib_list.h"
#include "bmuxlib_input.h"
#include "bkni_multi.h"
#include "bmuxlib_ts_mcpb.h"
#include "bmma_range.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/**********/
/* Handle */
/**********/
typedef struct BMUXlib_TS_Legacy_P_Context *BMUXlib_TS_Legacy_Handle;

#define BMUXLIB_TS_P_MAX(a,b) ((a > b) ? a : b)
#define BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP(a,b) ((b)?(((a) + ((b)-1))/(b)):0)

/**************/
/* Prototypes */
/**************/
void
BMUXlib_TS_Legacy_GetDefaultCreateSettings(
         BMUXlib_TS_CreateSettings *pCreateSettings
         );

BERR_Code
BMUXlib_TS_Legacy_Create(
         BMUXlib_TS_Legacy_Handle *phMuxTS,  /* [out] TSMuxer handle returned */
         const BMUXlib_TS_CreateSettings *pstCreateSettings
         );

void
BMUXlib_TS_Legacy_Destroy(
         BMUXlib_TS_Legacy_Handle hMuxTS
         );

/****************/
/* Mux Settings */
/****************/
BERR_Code
BMUXlib_TS_Legacy_SetMuxSettings(
         BMUXlib_TS_Legacy_Handle hMuxTS,
         const BMUXlib_TS_MuxSettings *pstMuxSettings
         );

BERR_Code
BMUXlib_TS_Legacy_GetMuxSettings(
         BMUXlib_TS_Legacy_Handle hMuxTS,
         BMUXlib_TS_MuxSettings *pstMuxSettings
         );

/**************/
/* Start/Stop */
/**************/
/* BMUXlib_TS_Start - Configures the mux HW */
BERR_Code
BMUXlib_TS_Legacy_Start(
         BMUXlib_TS_Legacy_Handle hMuxTS,
         const BMUXlib_TS_StartSettings *pstStartSettings
         );

BERR_Code
BMUXlib_TS_Legacy_Finish(
         BMUXlib_TS_Legacy_Handle hMuxTS,
         const BMUXlib_TS_FinishSettings *pstFinishSettings
         );

BERR_Code
BMUXlib_TS_Legacy_Stop(
         BMUXlib_TS_Legacy_Handle hMuxTS
         );

/**********/
/* Memory */
/**********/
void
BMUXlib_TS_Legacy_GetMemoryConfig(
         const BMUXlib_TS_MuxConfig *pstMuxConfig,
         BMUXlib_TS_MemoryConfig *pstMemoryConfig
         );

/***************/
/* System Data */
/***************/
BERR_Code
BMUXlib_TS_Legacy_AddSystemDataBuffers(
         BMUXlib_TS_Legacy_Handle hMuxTS,
         const BMUXlib_TS_SystemData *astSystemDataBuffer, /* Array of system data buffers */
         size_t uiCount, /* Count of system data buffers in array */
         size_t *puiQueuedCount /* Count of system data buffers queued by muxer (*puiQueuedCount <= uiCount) */
         );

BERR_Code
BMUXlib_TS_Legacy_GetCompletedSystemDataBuffers(
         BMUXlib_TS_Legacy_Handle hMuxTS,
         size_t *puiCompletedCount /* Returns count of system data buffers fully muxed */
         );

void
BMUXlib_TS_Legacy_GetStatus(
   BMUXlib_TS_Legacy_Handle hMuxTS,
   BMUXlib_TS_Status *pstStatus
   );

/***********/
/* Execute */
/***********/
BERR_Code
BMUXlib_TS_Legacy_DoMux(
   BMUXlib_TS_Legacy_Handle hMuxTS,
   BMUXlib_DoMux_Status *pstStatus
   );

/* NOTE: MUX treats a physical offset of zero as "invalid" (since its extremely unlikely to
   occur in reality) as a way of tracking a physical offset that has not been set
   (and is still at "defaults") */
#define BMUXLIB_TS_P_INVALID_OFFSET       BMUXLIB_INPUT_INVALID_OFFSET

#define BMUXLIB_TS_P_SCALE_MS_TO_27MHZ    (27000000 / 1000)

#define BMUXLIB_TS_P_FIXED_MUX_SERVICE_PERIOD     1
#define BMUXLIB_TS_P_PES_HEADER_MIN_PAYLOAD_SIZE  3
#define BMUXLIB_TS_P_A2PDELAY_DEFAULT             (2000 * BMUXLIB_TS_P_SCALE_MS_TO_27MHZ)  /* 2 seconds @ 27Mhz */
#define BMUXLIB_TS_P_PCR_ROLLBACK_INTERVALS       3 /* this must be at least 2 (required number of PCRs prior to A/V Data) */

/* DEBUG */
#if ( BMUXLIB_TS_P_DUMP_TRANSPORT_DESC || BMUXLIB_TS_P_DUMP_TRANSPORT_PES || BMUXLIB_TS_P_DUMP_PCR || BMUXLIB_TS_P_TEST_MODE)
#include <stdio.h>
#endif

#define BMUXLIB_TS_P_MOD300_WRAP(x)    (((((x) >> 32) % 0x258) << 32) | ((x) & 0xFFFFFFFF))
#define BMUXLIB_TS_P_MOD300_ADD32(_ui64Timestamp, _ui32Offset) BMUXLIB_TS_P_MOD300_WRAP((_ui64Timestamp) + (_ui32Offset));
#define BMUXLIB_TS_P_MOD300_SUB32(_ui64Timestamp, _ui32Offset) ( (_ui64Timestamp) >= (_ui32Offset) ) ? ((_ui64Timestamp) - (_ui32Offset)) : (((uint64_t)0x257 << 32 ) | (((_ui64Timestamp) - (_ui32Offset)) & 0xFFFFFFFF ));
#define BMUXLIB_TS_P_GET_PCR_BASE(x)   ((((((x) / 300) >> 32) & 0x1 ) << 32 ) | (((x) / 300) & 0xFFFFFFFF))
#define BMUXLIB_TS_P_GET_PCR_EXT(x)    (((x) % 300) & 0x1FF)

/* accessor macros for use in testing to manipulate mux state */
#define BMUXLIB_TS_P_GET_MUX_STATE(handle)         ((handle)->pstStatus->eState)
#define BMUXLIB_TS_P_SET_MUX_STATE(handle, state)  ((handle)->pstStatus->eState = (state))

/* accessors for sys data completed count */
#define BMUXLIB_TS_P_GET_SYS_DATA_COMP_CNT(handle)          ((handle)->pstStatus->uiSystemDataCompletedCount)
#define BMUXLIB_TS_P_SET_SYS_DATA_COMP_CNT(handle, count)   ((handle)->pstStatus->uiSystemDataCompletedCount = (count))

/* accessor macros for ESCR */
#define BMUXLIB_TS_P_INPUT_DESCRIPTOR_IS_ESCR_VALID(_inputDesc, _useDts) ( (_useDts) ? BMUXLIB_INPUT_DESCRIPTOR_IS_DTS_VALID(_inputDesc) : BMUXLIB_INPUT_DESCRIPTOR_IS_ESCR_VALID(_inputDesc) )

/**************************/
/* PES Header Definitions */
/**************************/

#define BMUXlib_TS_P_PESHeader_MINSIZE 9
#define BMUXlib_TS_P_PESHeader_MAXSIZE 19
typedef union BMUXlib_TS_P_PESHeader
{
      uint8_t data[BMUXlib_TS_P_PESHeader_MAXSIZE]; /* PES Header is 19 bytes */
#if 0
      struct {
            uint32_t uiStreamID:8;
            uint32_t uiStartCode:24; /* 000001h */

            uint32_t bPESExtensionFlag:1;
            uint32_t bPESCRCFlag:1;
            uint32_t bAdditionalCopyInfoFlag:1;
            uint32_t bDSMTrickModeFlag:1;
            uint32_t bESRateFlag:1;
            uint32_t bESCRFlag:1;
            uint32_t bDtsFlag:1;
            uint32_t bPtsFlag:1;
            uint32_t bOriginalFlag:1;
            uint32_t bCopyrightFlag:1;
            uint32_t bDataAlignmentIndicator:1;
            uint32_t bPESPriority:1;
            uint32_t uiPESScramblingControl:2;
            uint32_t uiExtensionCode:2; /* Always 10b */
            uint32_t uiPESPacketLength:16;

            uint8_t uiPESHeaderDataLength;

            uint32_t
      } fields;
#endif
} BMUXlib_TS_P_PESHeader;

#define BMUXlib_TS_P_PESHeader_SetStreamID(_pesHeader, _streamID) \
         _pesHeader[3] = ( _streamID & 0xFF );

#define BMUXlib_TS_P_PESHeader_SetPTS(_pesHeader, _pts) \
         _pesHeader[13] = ( ( ( _pts << 1  ) & 0xFE ) | (0x01) ); \
         _pesHeader[12] = (   ( _pts >> 7  ) & 0xFF ); \
         _pesHeader[11] = ( ( ( _pts >> 14 ) & 0xFE ) | (0x01) ); \
         _pesHeader[10] = (   ( _pts >> 22 ) & 0xFF ); \
         _pesHeader[9] =  ( ( ( _pts >> 29 ) & 0x0E ) | (0x21) ); \
         _pesHeader[7] |= 0x80; \
         BMUXlib_TS_P_PESHeader_AddHeaderLength(_pesHeader, 5 );

#define BMUXlib_TS_P_PESHeader_SetDTS(_pesHeader, _pts) \
         _pesHeader[18] = ( ( ( _pts << 1  ) & 0xFE ) | (0x01) ); \
         _pesHeader[17] = (   ( _pts >> 7  ) & 0xFF ); \
         _pesHeader[16] = ( ( ( _pts >> 14 ) & 0xFE ) | (0x01) ); \
         _pesHeader[15] = (   ( _pts >> 22 ) & 0xFF ); \
         _pesHeader[14] = ( ( ( _pts >> 29 ) & 0x0E ) | (0x11) ); \
         _pesHeader[9] |= (0x30); \
         _pesHeader[7] |= 0x40; \
         BMUXlib_TS_P_PESHeader_AddHeaderLength(_pesHeader, 5 );

#define BMUXlib_TS_P_PESHeader_SetLength(_pesHeader, _length) \
         _pesHeader[4] = ( ( ( _length ) >> 8 ) & 0xFF ); \
         _pesHeader[5] = ( ( ( _length ) )      & 0xFF );

#define BMUXlib_TS_P_PESHeader_AddHeaderLength(_pesHeader, _headerLength ) \
         (_pesHeader)[8] += (_headerLength);

#define BMUXlib_TS_P_PESHeader_GetHeaderLength(_pesHeader ) \
         (_pesHeader)[8]

/*************************/
/* TS Packet Definitions */
/*************************/
#define BMUXlib_TS_P_TSPacket_MAXSIZE 188
typedef union BMUXlib_TS_P_TSPacket
{
      uint8_t data[BMUXlib_TS_P_TSPacket_MAXSIZE]; /* TS Packet is 188 bytes */
} BMUXlib_TS_P_TSPacket;

#define BMUXlib_TS_P_TSPacket_SetPayloadUnitStart(_tsPacket, _bPayloadUnitStart) \
   ((uint8_t*)_tsPacket)[1] = ( _bPayloadUnitStart ? 0x40 : 0x00 ) | ( ((uint8_t*)_tsPacket)[1] & ~0x40);

#define BMUXlib_TS_P_TSPacket_SetPID(_tsPacket, _pid) \
   ((uint8_t*)_tsPacket)[1] = ( ( _pid >> 8 ) & 0x1F ) | ( ((uint8_t*)_tsPacket)[1] & ~0x1F ); \
   ((uint8_t*)_tsPacket)[2] = ( ( _pid      ) & 0xFF );

#define BMUXlib_TS_P_TSPacket_SetAdaptationPresent(_tsPacket, _bAdaptationPresent) \
   ((uint8_t*)_tsPacket)[3] = ( _bAdaptationPresent ? 0x20 : 0x00 ) | (((uint8_t*)_tsPacket)[3] & ~0x20 );

#define BMUXlib_TS_P_TSPacket_SetPayloadPresent(_tsPacket, _bPayloadPresent) \
   ((uint8_t*)_tsPacket)[3] = ( _bPayloadPresent ? 0x10 : 0x00 ) | (((uint8_t*)_tsPacket)[3] & ~0x10 );

#define BMUXlib_TS_P_TSPacket_SetContinuityCounter(_tsPacket, _ContinuityCounter) \
   ((uint8_t*)_tsPacket)[3] = ( _ContinuityCounter & 0x0F ) | (((uint8_t*)_tsPacket)[3] & ~0x0F );

extern const BMUXlib_TS_P_TSPacket s_stDefaultTSPacket;
extern const BMUXlib_TS_P_TSPacket s_stNullTSPacket;

/**************************/
/* PCR Packet Definitions */
/**************************/
#define BMUXlib_TS_P_TSPacket_SetAdaptationLength(_tsPacket, _AdaptationLength) \
   ((uint8_t*)_tsPacket)[4] = _AdaptationLength;   \
   if (_AdaptationLength != 0) ((uint8_t*)_tsPacket)[5] = 0

#define BMUXlib_TS_P_TSPacket_SetPCRPresent(_tsPacket, _bPCRPresent) \
   ((uint8_t*)_tsPacket)[5] = ( _bPCRPresent ? 0x10 : 0x00 ) | ( ((uint8_t*)_tsPacket)[5] & ~0x10 );

#define BMUXlib_TS_P_TSPacket_SetPCRBase( _tsPacket, _PCRBase ) \
   ((uint8_t*)_tsPacket)[ 6] = ( (_PCRBase >> 25 ) & 0xFF ); \
   ((uint8_t*)_tsPacket)[ 7] = ( (_PCRBase >> 17 ) & 0xFF ); \
   ((uint8_t*)_tsPacket)[ 8] = ( (_PCRBase >>  9 ) & 0xFF ); \
   ((uint8_t*)_tsPacket)[ 9] = ( (_PCRBase >>  1 ) & 0xFF ); \
   ((uint8_t*)_tsPacket)[10] = ( (_PCRBase <<  7 ) & 0x80 ) | ( ((uint8_t*)_tsPacket)[10] & 0x01 ) | 0x7E;

#define BMUXlib_TS_P_TSPacket_SetPCRExtension( _tsPacket, _PCRExtension ) \
   ((uint8_t*)_tsPacket)[10] = ( (_PCRExtension >>  8 ) & 0xFF ) | (((uint8_t*)_tsPacket)[10] & 0x80 ) | 0x7E; \
   ((uint8_t*)_tsPacket)[11] = ( (_PCRExtension       ) & 0xFF ); \

/**************************/
/* BPP Data Definitions */
/**************************/
#define BMUXlib_TS_P_BPPData_MAXSIZE 184
typedef union BMUXlib_TS_P_BPPData
{
      uint8_t data[BMUXlib_TS_P_BPPData_MAXSIZE]; /* BPP Packet is 184 bytes */
} BMUXlib_TS_P_BPPData;

#define BMUXlib_TS_P_MTUBPPData_NUMCONSECUTIVEPACKETS 16
#define BMUXlib_TS_P_MTUBPPData_MAXSIZE BMUXlib_TS_P_BPPData_MAXSIZE*BMUXlib_TS_P_MTUBPPData_NUMCONSECUTIVEPACKETS
typedef union BMUXlib_TS_P_MTUBPPData
{
      uint8_t data[BMUXlib_TS_P_MTUBPPData_MAXSIZE]; /* BPP Packet is 184 bytes */
} BMUXlib_TS_P_MTUBPPData;

#define BMUXlib_TS_P_BPPData_SetStreamID(_bppData, _streamID) \
         ((uint8_t*)(_bppData))[3] = ( _streamID & 0xFF );

#define BMUXlib_TS_P_BPPData_SetCommand(_bppData, _command) \
         ((uint8_t*)(_bppData))[30] = ( ((_command) >> 24) & 0xFF ); \
         ((uint8_t*)(_bppData))[31] = ( ((_command) >> 16) & 0xFF ); \
         ((uint8_t*)(_bppData))[32] = ( ((_command) >> 8 ) & 0xFF ); \
         ((uint8_t*)(_bppData))[33] = ( ((_command) )      & 0xFF );

#define BMUXlib_TS_P_BPPData_SetControlWord(_bppData, _word, _value) \
         ((uint8_t*)(_bppData))[30+((_word)*4)] = ( ((_value) >> 24) & 0xFF ); \
         ((uint8_t*)(_bppData))[31+((_word)*4)] = ( ((_value) >> 16) & 0xFF ); \
         ((uint8_t*)(_bppData))[32+((_word)*4)] = ( ((_value) >> 8 ) & 0xFF ); \
         ((uint8_t*)(_bppData))[33+((_word)*4)] = ( ((_value) )      & 0xFF );

extern const BMUXlib_TS_P_BPPData s_stDefaultBPPData;
extern const BMUXlib_TS_P_BPPData s_stDummyPESFrame;

/************************/
/* BTP Data Definitions */
/************************/
#define BMUXlib_TS_P_MUX_TIMESTAMP_UPDATE_COMMAND 0x18
#define BMUXlib_TS_P_LAST_COMMAND 0x82
#define BMUXlib_TS_P_MUX_TIMESTAMP_UPDATE_COMMAND_OFFSET 0
#define BMUXlib_TS_P_MUX_TIMESTAMP_UPDATE_TIMESTAMP_CTRL_OFFSET 7
#define BMUXLIB_TS_P_MUX_TIMESTAMP_UPDATE_TIMESTAMP_CTRL_SET_PUSI_IN_NEXT_PKT 0x4
#define BMUXLIB_TS_P_MUX_TIMESTAMP_UPDATE_TIMESTAMP_CTRL_SET_ADAPTATION_FIELD_TO_3 0x8

#define BMUXlib_TS_P_MUX_TIMESTAMP_UPDATE_PKT2PKT_DELTA_TIMESTAMP_OFFSET 8
#define BMUXlib_TS_P_MUX_TIMESTAMP_UPDATE_NEXT_PKT_TIMESTAMP_OFFSET 9

#define BMUXlib_TS_P_BTPData_SetControlWord(_btpData, _word, _value) \
         ((uint8_t*)(_btpData))[12+((_word)*4)] = ( ((_value) >> 24) & 0xFF ); \
         ((uint8_t*)(_btpData))[13+((_word)*4)] = ( ((_value) >> 16) & 0xFF ); \
         ((uint8_t*)(_btpData))[14+((_word)*4)] = ( ((_value) >> 8 ) & 0xFF ); \
         ((uint8_t*)(_btpData))[15+((_word)*4)] = ( ((_value) )      & 0xFF );

extern const BMUXlib_TS_P_TSPacket s_stDefaultBTPPacket;

/******************************/
/* Mux TS Context Definitions */
/******************************/
typedef enum BMUXlib_TS_P_DataType
{
      BMUXlib_TS_P_DataType_eCDB,   /* this used for external inputs (i.e. video, audio, sys data, userdata) */
      BMUXlib_TS_P_DataType_ePESHeader,
      BMUXlib_TS_P_DataType_eTSPacket,
      BMUXlib_TS_P_DataType_eBPP,
      BMUXlib_TS_P_DataType_eNULL,
      BMUXlib_TS_P_DataType_eUserdataPTS,    /* a PTS entry for userdata packet timing updates */
      BMUXlib_TS_P_DataType_eUserdataLocal,  /* local storage used for "unwrapping" userdata TS packets */
      BMUXlib_TS_P_DataType_eMTUBPP,

      BMUXlib_TS_P_DataType_eMax
} BMUXlib_TS_P_DataType;

typedef struct BMUXlib_TS_P_InputMetaData
{
   BMUXlib_Input_Handle hInput;
   unsigned uiInputIndex;
   unsigned uiTransportChannelIndex;   /* Which transport channel interface to use for this PID */
   uint16_t uiPID;                     /* The PID associated with this input data */
   uint8_t uiPESStreamID;
   unsigned uiPIDIndex;

   /* State */
   bool bEOSSeen;  /* true if EOS descriptor has been seen */

   unsigned uiMetadataDescriptorsSkipped; /* Number of source metdata descriptors skipped */
#if ( BMUXLIB_TS_P_DUMP_VIDEO_DESC | BMUXLIB_TS_P_DUMP_AUDIO_DESC )
   FILE *hDescDumpFile;
   bool bPrintedHeader;
#endif

   struct BMUXlib_TS_P_UserdataVideoInfo *pstUserdata;   /* information required by userdata related to this input (video only) */

   BAVC_FrameRateCode eSourceFrameRateCode;              /* used by userdata for dDTS verification */

   unsigned uiPreviousPacket2PacketTimestampDelta;
   unsigned uiCurrentSegmentCount;
} BMUXlib_TS_P_InputMetaData;

#include "bmuxlib_ts_userdata.h"

/* if data type is CDB, this defines the source of the data */
typedef enum BMUXlib_TS_P_SourceType
{
      BMUXlib_TS_P_SourceType_eUnknown,
      BMUXlib_TS_P_SourceType_eVideo,
      BMUXlib_TS_P_SourceType_eAudio,
      BMUXlib_TS_P_SourceType_eSystem,
      BMUXlib_TS_P_SourceType_eUserdata,

      BMUXlib_TS_P_SourceType_eMax
} BMUXlib_TS_P_SourceType;

#define BMUXLib_TS_P_STATS_MAX_MSP_COUNT 120
typedef struct BMUXLib_TS_P_EfficiencyStats
{
   uint32_t uiTotalBytesPerInput[BMUXlib_TS_P_DataType_eMax][BMUXlib_TS_P_SourceType_eMax];
   uint32_t uiTotalBytesPerDataType[BMUXlib_TS_P_DataType_eMax];
   uint32_t uiTotalBytesPerSourceType[BMUXlib_TS_P_SourceType_eMax];
   uint32_t uiTotalBytes;
   uint64_t uiTotalBytesWritten;

   uint16_t uiTimeInMs[BMUXLib_TS_P_STATS_MAX_MSP_COUNT];
   uint32_t uiTotalTimeInMs;
   uint32_t uiNumBytes[BMUXlib_TS_P_DataType_eMax][BMUXlib_TS_P_SourceType_eMax][BMUXLib_TS_P_STATS_MAX_MSP_COUNT];
   unsigned uiIndex;

   uint32_t uiPacketizationOverhead[BMUXLib_TS_P_STATS_MAX_MSP_COUNT];
   uint32_t uiFrameSize[BMUXlib_TS_P_SourceType_eMax];
} BMUXLib_TS_P_EfficiencyStats;

BMUXLIB_LIST_DEFINE( BMUXlib_TS_P_PESHeader )
BMUXLIB_LIST_DEFINE( BMUXlib_TS_SystemData )
BMUXLIB_LIST_DEFINE( BMUXlib_TS_P_TSPacket )
BMUXLIB_LIST_DEFINE( BMUXlib_TS_P_BPPData )
BMUXLIB_LIST_DEFINE( BMUXlib_TS_P_MTUBPPData )
BMUXLIB_LIST_DEFINE( BMUXlib_TS_P_UserdataPending )
BMUXLIB_LIST_DEFINE( BMUXlib_TS_P_UserdataPTSEntry )

#define BMUXLIB_TS_TRANSPORT_IS_FULL( _pmux, _index ) ( BMUXLIB_LIST_COUNT ( &(_pmux)->stTransportDescriptorPendingList[_index] ) >= (_pmux)->pstStatus->aTransportDescriptorPendingListCountTable[_index] )
#define BMUXLIB_TS_TRANSPORT_GET_NUM_FREE( _pmux, _index, _pcount ) ( *(_pcount) = ((_pmux)->pstStatus->aTransportDescriptorPendingListCountTable[_index] - BMUXLIB_LIST_COUNT ( &(_pmux)->stTransportDescriptorPendingList[_index] ) ) )

typedef struct BMUXlib_TS_P_TransportDescriptorMetaData
{
      BMUXlib_TS_P_DataType eDataType;
      uint32_t uiSourceDescriptorCount; /* Set to one if the corresponding source descriptor should be freed when this transport descriptor is done */

      BMUXlib_TS_P_SourceType eSourceType;
      uint64_t uiTimestamp;
      /* The following are only valid for audio/video/userdata source types */
      unsigned uiInputIndex;
      uint32_t uiSequenceID;     /* SW7425-3250: Sequence ID for Userdata Release Q */
      struct
      {
         bool bValid;
         uint32_t uiDtsIn27Mhz;
      } stDtsInfo;

      /* uiBufferBaseOffset is obtained through LockOffset when hBufferBaseBlock is set,
      and unlocked when hBufferBaseBlock is cleared.
      Enforced by bmuxlib_p_lock_metadata_memory and bmuxlib_p_unlock_metadata_memory.
      It should never be on stTransportDescriptorFreeList with locked memory. */
      BMMA_Block_Handle hBufferBaseBlock;
      BSTD_DeviceOffset uiBufferBaseOffset;
      void *pBufferAddress;
      union
      {
         BMUXLIB_LIST_ENTRY_TYPE( BMUXlib_TS_P_PESHeader ) *pPESHeader;
         BMUXLIB_LIST_ENTRY_TYPE( BMUXlib_TS_SystemData ) *pSystemData;
         BMUXLIB_LIST_ENTRY_TYPE( BMUXlib_TS_P_TSPacket ) *pTSPacket;
         BMUXLIB_LIST_ENTRY_TYPE( BMUXlib_TS_P_BPPData ) *pBPPData;
         BMUXLIB_LIST_ENTRY_TYPE( BMUXlib_TS_P_MTUBPPData ) *pMTUBPPData;
         BMUXLIB_LIST_ENTRY_TYPE( BMUXlib_TS_P_UserdataPending ) *pUserDataPending;
         BMUXLIB_LIST_ENTRY_TYPE( BMUXlib_TS_P_UserdataPTSEntry ) *pUserDataPTSEntry;
      } uListEntry;
} BMUXlib_TS_P_TransportDescriptorMetaData;

typedef struct BMUXlib_TS_P_TransportDescriptorEntry
{
   BLST_SQ_ENTRY(BMUXlib_TS_P_TransportDescriptorEntry) link;

   BMUXlib_TS_TransportDescriptor stData;
   BMUXlib_TS_P_TransportDescriptorMetaData stMetaData;
} BMUXlib_TS_P_TransportDescriptorEntry;

BMUXLIB_LIST_DEFINE_HEAD( BMUXlib_TS_P_TransportDescriptor )
BMUXLIB_LIST_DEFINE_LIST( BMUXlib_TS_P_TransportDescriptor )

typedef struct BMUXlib_TS_P_PCRInfo
{
      bool bInitialized;
      bool bEarliestESCRValid;
      bool bEarliestPTSValid;
      uint64_t uiBase;
      uint16_t uiExtension;
      uint64_t uiESCR;
      uint64_t uiNextESCR;
      uint64_t uiEarliestPTS;
      uint32_t uiEarliestESCR;
      uint32_t uiIntervalIn27Mhz;
#ifdef BMUXLIB_TS_P_DUMP_PCR
      FILE *hPCRFile;
#endif
} BMUXlib_TS_P_PCRInfo;

typedef struct BMUXlib_TS_P_SystemDataInfo
{
      bool bLastScheduledESCRValid;
      uint32_t uiESCR;
      uint32_t uiPacketsMuxedSoFar;
      uint32_t uiPacketsUntilNextPCR;
      uint32_t uiLastScheduledESCR;
#ifdef BMUXLIB_TS_P_TEST_MODE
      FILE *hCSVFile;         /* system data information for each "block" written */
      FILE *hDataFile;        /* binary system data written here */
      bool bCSVOpened;
#endif
} BMUXlib_TS_P_SystemDataInfo;

typedef enum BMUXlib_TS_P_MemoryType
{
   BMUXlib_TS_P_MemoryType_eSystem, /* Only accessed by the host */
   BMUXlib_TS_P_MemoryType_eShared, /* Accessed by both the host and device(s) */

   BMUXlib_TS_P_MemoryType_eMax
} BMUXlib_TS_P_MemoryType;

typedef struct BMUXlib_TS_P_BufferInfo
{
   size_t uiSize;
   void *pBuffer;
   BSTD_DeviceOffset uiBufferOffset;
} BMUXlib_TS_P_BufferInfo;

typedef struct BMUXlib_TS_P_BufferConfig
{
   BMUXlib_TS_P_BufferInfo stBufferInfo[BMUXlib_TS_P_MemoryType_eMax];
} BMUXlib_TS_P_BufferConfig;

typedef enum BMUXlib_TS_P_InputType
{
   BMUXlib_TS_P_InputType_eAudio,
   BMUXlib_TS_P_InputType_eVideo,
   BMUXlib_TS_P_InputType_eSystem,

   BMUXlib_TS_P_InputType_eMax
} BMUXlib_TS_P_InputType;

typedef enum BMUXlib_TS_P_MemoryEntryType
{
   BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor, /* A/V/S */
   BMUXlib_TS_P_MemoryEntryType_ePESHeader, /* A/V */
   BMUXlib_TS_P_MemoryEntryType_eTransportPacket, /* S-only (for PCR and BTP) */
   BMUXlib_TS_P_MemoryEntryType_eSystemData, /* S-only (for system data) */
   BMUXlib_TS_P_MemoryEntryType_eBPP, /* A/V */
   BMUXlib_TS_P_MemoryEntryType_eUserData, /* S-only (for user data) */
   BMUXlib_TS_P_MemoryEntryType_eUserDataUnwrap, /* S-only (for user data) */
   BMUXlib_TS_P_MemoryEntryType_eUserDataReleaseQ, /* S-only (for user data) */
   BMUXlib_TS_P_MemoryEntryType_eUserDataPTS, /* S-only (for user data) */
   BMUXlib_TS_P_MemoryEntryType_eMTUBPP, /* A/V */

   BMUXlib_TS_P_MemoryEntryType_eMax
} BMUXlib_TS_P_MemoryEntryType;

typedef struct BMUXlib_TS_P_MemoryEntry
{
   size_t uiCount; /* Number of entries */
   BMUXlib_TS_P_BufferConfig stMemoryConfig;  /* Total Size for all entries */
} BMUXlib_TS_P_MemoryEntry;

typedef struct BMUXlib_TS_P_MemoryConfigTotal
{
   BMUXlib_TS_P_MemoryEntry astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eMax];

   BMUXlib_TS_P_BufferConfig stMemoryConfig; /* Contains the number of bytes required to allocate the items listed above */
} BMUXlib_TS_P_MemoryConfigTotal;

typedef struct BMUXlib_TS_P_MemoryConfig
{
   BMUXlib_TS_P_MemoryEntry astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eMax][BMUXlib_TS_P_InputType_eMax];

   BMUXlib_TS_P_BufferConfig stMemoryConfig;
} BMUXlib_TS_P_MemoryConfig;

typedef struct BMUXlib_TS_P_MemoryBuffers
{
   void* pSystemBuffer; /* Memory only accessed by the host. If NULL, allocated in BMUXlib_TS_Create() from system/kernel memory. */
   void* pSharedBuffer; /* Memory accessed by both the host and device(s). If NULL, allocated in BMUXlib_TS_Create() from specified hMem. */
} BMUXlib_TS_P_MemoryBuffers;

typedef struct BMUXlib_TS_P_Input_Info
{
   unsigned uiTransportChannelIndex;
} BMUXlib_TS_P_Input_Info;


typedef struct BMUXlib_TS_P_Output_Info
{
   struct
   {
      BMUXlib_TS_MCPB_Handle hMuxMCPB;
      BMUXlib_TS_MCPB_Channel_Handle ahMuxMCPBCh[BMUXLIB_TS_MAX_VIDEO_PIDS + BMUXLIB_TS_MAX_AUDIO_PIDS + 1];
      unsigned uiNumOpenChannels;
   } stMCPB[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES]; /* This index is the original uiTransportChannelIndex provided in StartSettings */

   struct
   {
      bool bActive;
      bool bStalled;

      struct
      {
         uint32_t uiNextExpectedESCR;
         uint32_t uiLastStartingESCR;
         uint32_t uiCurrentPacket2PacketTimestampDelta;
      } stTimingPending, stTimingQueued;

#if ( BMUXLIB_TS_P_DUMP_TRANSPORT_DESC || BMUXLIB_TS_P_DUMP_TRANSPORT_PES )
      struct
      {
#if BMUXLIB_TS_P_DUMP_TRANSPORT_DESC
      FILE *hDescDumpFile;
      bool bPrintedHeader;
#endif
#if BMUXLIB_TS_P_DUMP_TRANSPORT_PES
      FILE *hPESDumpFile;
#endif
      } stDebug;
#endif

      uint32_t uiDescriptorsQueued; /* Number of descriptors currently pending/queued in transport */
      uint32_t uiDescriptorsSent; /* Number of descriptors sent to transport total */

      struct
      {
         bool bValid;
         BAVC_TsMux_DescConfig stTsMuxDescriptorConfig;
         unsigned uiPacingCounterIn27Mhz;
         size_t uiLengthOfFrame;
         bool bDtsValid;
         uint32_t uiDtsIn27Mhz;
      } stUnderflowDetection;

      struct
      {
         bool bValid;
         unsigned uiValue;
      } stPacket2PacketDelta;

      BMUXlib_TS_TransportChannelInterface stChannelInterface;
   } stTransport[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES]; /* This index is the mapped uiTransportChannelIndex assigned to the input */

   unsigned uiNumTransportChannelsOpen;
} BMUXlib_TS_P_Output_Info;

#define BMUXLIB_TS_PID_MAX_ENTRIES 8192
#define BMUXLIB_TS_PID_PER_ENTRY (sizeof(uint32_t) * 8)
#define BMUXLIB_TS_PID_TABLE_SIZE (BMUXLIB_TS_PID_MAX_ENTRIES/BMUXLIB_TS_PID_PER_ENTRY)
#define BMUXLIB_TS_PID_ENTRY_SET(_pidTable, _pid) _pidTable[(_pid)/BMUXLIB_TS_PID_PER_ENTRY] |= (1 << ((_pid)%BMUXLIB_TS_PID_PER_ENTRY))
#define BMUXLIB_TS_PID_ENTRY_CLR(_pidTable, _pid) _pidTable[(_pid)/BMUXLIB_TS_PID_PER_ENTRY] &= ~(1 << ((_pid)%BMUXLIB_TS_PID_PER_ENTRY))
#define BMUXLIB_TS_PID_ENTRY_IS_SET(_pidTable, _pid) (0 != (_pidTable[(_pid)/BMUXLIB_TS_PID_PER_ENTRY] & (1 << ((_pid)%BMUXLIB_TS_PID_PER_ENTRY))))

typedef struct BMUXlib_TS_P_Status
{
   BMUXlib_State eState;
   bool bTransportConfigured; /* Set to true when initialization is complete (E.g. Initial PCR Value seeded, transport is configured, etc.) */

   BMUXlib_TS_P_PCRInfo stPCRInfo;
   BMUXlib_TS_P_SystemDataInfo stSystemDataInfo;
   bool bFirstESCRValid;
   uint32_t uiFirstESCR;

   bool bTransportStatusValid;
   BMUXlib_TS_TransportStatus stTransportStatus;
   BMUXlib_TS_TransportStatus stPreviousTransportStatus;

   BMUXlib_TS_TransportSettings stTransportSettings;

   /* Metadata */
   BMUXlib_TS_P_InputMetaData stInputMetaData[BMUXLIB_TS_MAX_VIDEO_PIDS + BMUXLIB_TS_MAX_AUDIO_PIDS];
   unsigned uiNumInputs;
   bool bAllInputsReady;
   bool bWaitForAllInputs;

   /* System Data */
   uint32_t uiSystemDataCompletedCount;

   BMUXlib_DoMux_Status stDoMuxStatus;

   BMUXlib_TS_P_MemoryConfig stMemoryConfig;
   BMUXlib_TS_P_MemoryConfigTotal stMemoryConfigTotal;

   unsigned uiPreviousESCR;
   unsigned uiPreviousPacket2PacketTimestampDelta;
   unsigned uiTotalPacket2PacketTimestampDelta;
   uint32_t uiTotalPacket2PacketTimestampDeltaMod300; /* 23 bits base + 9 bits extension */
   bool bBPPSentForVideo;
   bool bBTPSent;

   BMUXLIB_LIST_ENTRY_TYPE(BMUXlib_TS_P_TSPacket) *pNullTSPacketBuffer;
   BMUXLIB_LIST_ENTRY_TYPE(BMUXlib_TS_P_BPPData) *pDummyPESBuffer;

   uint32_t uiLastESCRDTSDelta;  /* SW7425-4340: used primarily for debugging A2PDelay */

   int32_t iExecutionTimeAdjustment; /* SW7425-4707: used to prevent late ESCRs when descriptors are deferred to the next MSP */

   struct
   {
      unsigned uiVideo[BMUXLIB_TS_MAX_VIDEO_PIDS];
      unsigned uiAudio[BMUXLIB_TS_MAX_AUDIO_PIDS];
   } stInputIndexLUT;

   struct
   {
      unsigned uiDescriptorsAdded;
      unsigned uiDescriptorsCompleted;
      unsigned uiDescriptorsReturned;
   } stTransport[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES];
#if BMUXLIB_TS_P_ENABLE_STATS
   BMUXLib_TS_P_EfficiencyStats stEfficiencyStats;
#endif

   struct
   {
      BMUXlib_TS_P_Input_Info video[BMUXLIB_TS_MAX_VIDEO_PIDS];
      BMUXlib_TS_P_Input_Info audio[BMUXLIB_TS_MAX_AUDIO_PIDS];
      BMUXlib_TS_P_Input_Info system;
   } stInput;

   BMUXlib_TS_P_Output_Info stOutput;

   BMUXlib_TS_Status stStatus;

   unsigned uiCurrentSegmentCount;

   bool bTimingOffsetValid;
   uint32_t uiTimingOffsetIn27Mhz;
   uint64_t uiTimingOffsetIn90Khz;

   uint32_t aTransportDescriptorPendingListCountTable[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES];
   bool bSystemDataPreQ;
} BMUXlib_TS_P_Status;

typedef struct BMUXlib_TS_P_TempMemoryConfig
{
   BMUXlib_TS_P_MemoryConfig stMemoryConfig;
   BMUXlib_TS_MuxConfig stMuxConfig;
   BMMA_RangeAllocator_Status stRangeAllocatorStatus;
} BMUXlib_TS_P_TempMemoryConfig;

typedef struct BMUXlib_TS_P_UserdataStatusPriv
{
   /* userdata */
   BMUXlib_TS_P_UserdataStatus stUserdataStatus;
   BMUXlib_TS_P_UserdataVideoInfo stUserdataVideoInfo[BMUXLIB_TS_MAX_VIDEO_PIDS];   /* companion video info */
   BMUXlib_TS_P_UserdataInfo stUserdataInfo[BMUXLIB_TS_MAX_USERDATA_PIDS];
} BMUXlib_TS_P_UserdataStatusPriv;

typedef struct BMUXlib_TS_P_Settings
{
   /* Settings */
   BMUXlib_TS_MuxSettings stMuxSettings;

   /* Start Settings */
   BMUXlib_TS_StartSettings stStartSettings;

   /* Finish Settings */
   BMUXlib_TS_FinishSettings stFinishSettings;
} BMUXlib_TS_P_Settings;

typedef struct BMUXlib_TS_Legacy_P_Context
{
      BDBG_OBJECT(BMUXlib_TS_Legacy_P_Context)

      /* Create */
      BMUXlib_TS_CreateSettings stCreateSettings;
      BMUXlib_TS_P_MemoryBuffers stMemoryBuffers;

      BMMA_RangeAllocator_Block_Handle hTransportDescriptorTempBlock;
      BMUXlib_TS_TransportDescriptor *astTransportDescriptorTemp;
      BMMA_RangeAllocator_Block_Handle hTransportDescriptorMetaDataTempBlock;
      BMUXlib_TS_P_TransportDescriptorMetaData *astTransportDescriptorMetaDataTemp;
      BMMA_RangeAllocator_Block_Handle hTransportDescriptorBlock;
      BMUXLIB_LIST_ENTRY_TYPE(BMUXlib_TS_P_TransportDescriptor) *astTransportDescriptor;
      BMUXLIB_LIST_TYPE(BMUXlib_TS_P_TransportDescriptor) stTransportDescriptorFreeList; /* Stack containing free transport descriptors */
      BMUXLIB_LIST_TYPE(BMUXlib_TS_P_TransportDescriptor) stTransportDescriptorPendingList[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES]; /* FIFO containing the pending transport descriptors
                                                                                                     * for each transport interface.  Entries move between
                                                                                                     * the pending and free lists */

      unsigned uiPendingCompleted[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES];
      BMMA_RangeAllocator_Block_Handle hPESHeaderBlock;
      BMUXLIB_LIST_ENTRY_TYPE(BMUXlib_TS_P_PESHeader) *astPESHeader;
      BMUXLIB_LIST_TYPE(BMUXlib_TS_P_PESHeader) stPESHeaderFreeList; /* Stack containing free PES Header buffers */

      BMMA_RangeAllocator_Block_Handle hTSPacketBlock;
      BMUXLIB_LIST_ENTRY_TYPE(BMUXlib_TS_P_TSPacket) *astTSPacket;
      BMUXLIB_LIST_TYPE(BMUXlib_TS_P_TSPacket) stTSPacketFreeList; /* Stack containing TS Packet buffers */

      BMMA_RangeAllocator_Block_Handle hBPPDataBlock;
      BMUXLIB_LIST_ENTRY_TYPE(BMUXlib_TS_P_BPPData) *astBPPData; /* Used for BPP packets.  Can be reused within a playback. */
      BMUXLIB_LIST_TYPE(BMUXlib_TS_P_BPPData) stBPPFreeList; /* Stack containing BPP buffers */

      BMMA_RangeAllocator_Block_Handle hMTUBPPDataBlock;
      BMUXLIB_LIST_ENTRY_TYPE(BMUXlib_TS_P_MTUBPPData) *astMTUBPPData; /* Used for MTU BPP packets.  Cannot be reused within a playback. */
      BMUXLIB_LIST_TYPE(BMUXlib_TS_P_MTUBPPData) stMTUBPPFreeList; /* Stack containing MTU BPP buffers */

      BMUXLIB_LIST_TYPE(BMUXlib_TS_SystemData) stSystemDataPendingList;
      BMUXLIB_LIST_TYPE(BMUXlib_TS_SystemData) stSystemDataFreeList;
      BMMA_RangeAllocator_Block_Handle hSystemDataBlock;
      BMUXLIB_LIST_ENTRY_TYPE(BMUXlib_TS_SystemData) *astSystemData;

      BMUXLIB_LIST_TYPE(BMUXlib_TS_P_UserdataPending) stUserdataPendingList[BMUXLIB_TS_MAX_USERDATA_PIDS]; /* data pending for sending to transport for each userdata input */
      BMUXLIB_LIST_TYPE(BMUXlib_TS_P_UserdataPending) stUserdataFreeList;
      BMMA_RangeAllocator_Block_Handle hUserdataPendingBlock;
      BMUXLIB_LIST_ENTRY_TYPE(BMUXlib_TS_P_UserdataPending) *astUserdataPending;

      /* the memory used to store updated PTS values for userdata PES packets */
      BMUXLIB_LIST_TYPE(BMUXlib_TS_P_UserdataPTSEntry) stUserdataPTSFreeList;
      BMMA_RangeAllocator_Block_Handle hUserdataPTSBlock;
      BMUXLIB_LIST_ENTRY_TYPE(BMUXlib_TS_P_UserdataPTSEntry) *astUserdataPTS;

      /* memory used for "unwrapping" TS packets for sending to transport */
      BMMA_RangeAllocator_Block_Handle hUserdataUnwrapBlock;
      BMUXlib_TS_P_TSPacket *astUserdataUnwrap;

      /* NOTE: The following cannot use a "List" object since these do not support
         removal from arbitrary nodes */
      BMUXlib_TS_P_UserdataReleaseQEntry *pUserdataReleaseQFreeList;
      BMMA_RangeAllocator_Block_Handle hUserdataReleaseQFreeListBaseBlock;
      BMUXlib_TS_P_UserdataReleaseQEntry *pUserdataReleaseQFreeListBase;

      BMUXlib_Input_Handle ahInput[BMUXLIB_TS_MAX_VIDEO_PIDS + BMUXLIB_TS_MAX_AUDIO_PIDS];
      BMUXlib_InputGroup_Handle hInputGroup;

      struct
      {
         BMMA_Block_Handle hBlock;
         void *pBuffer;

         BMMA_RangeAllocator_CreateSettings stMmaRangeAllocatorCreateSettings;
         BMMA_RangeAllocator_Handle hMmaRangeAllocator;
      } stSubHeap[BMUXlib_TS_P_MemoryType_eMax];

      /* Status */
      BMUXlib_TS_P_Status *pstStatus;
      uint32_t *aFoundPIDs;  /* Table of flags indicating PIDs in use: 2^13 possible PIDs */
      BMUXlib_TS_P_UserdataStatusPriv *pstUserdataStatus;
      BMUXlib_TS_P_Settings *pstSettings;
} BMUXlib_TS_Legacy_P_Context;

/*************************/
/* Private Mux Functions */
/*************************/
BERR_Code
BMUXlib_TS_P_ConfigureTransport(
         BMUXlib_TS_Legacy_Handle hMuxTS
         );

BERR_Code
BMUXlib_TS_P_ProcessCompletedBuffers(
         BMUXlib_TS_Legacy_Handle hMuxTS
         );

BERR_Code
BMUXlib_TS_P_ProcessSystemData(
         BMUXlib_TS_Legacy_Handle hMuxTS
         );

BERR_Code
BMUXlib_TS_P_ProcessNewBuffers(
         BMUXlib_TS_Legacy_Handle hMuxTS
         );

BERR_Code
BMUXlib_TS_P_ScheduleProcessedBuffers(
         BMUXlib_TS_Legacy_Handle hMuxTS
         );

bool
BMUXlib_TS_P_Flush(
         BMUXlib_TS_Legacy_Handle hMuxTS,
         bool bFlushSystemData
         );

BERR_Code
BMUXlib_TS_P_AddTransportDescriptor(
         BMUXlib_TS_Legacy_Handle hMuxTS,
         unsigned uiTransportChannelIndex,
         BMUXlib_TS_P_TransportDescriptorEntry *pstTransportDescriptor
         );

BERR_Code
BMUXlib_TS_P_AddSystemDataBuffers(
         BMUXlib_TS_Legacy_Handle hMuxTS,
         const BMUXlib_TS_SystemData *astSystemDataBuffer, /* Array of system data buffers */
         size_t uiCount, /* Count of system data buffers in array */
         size_t *puiQueuedCount /* Count of system data buffers queued by muxer (*puiQueuedCount <= uiCount) */
         );

void
BMUXlib_TS_P_SeedPCR(
         BMUXlib_TS_Legacy_Handle hMuxTS
         );

uint64_t
BMUXLIB_TS_P_INPUT_DESCRIPTOR_PTS(
   BMUXlib_TS_Legacy_Handle hMuxTS,
   const BMUXlib_Input_Descriptor *pstDescriptor
   );

uint64_t
BMUXLIB_TS_P_INPUT_DESCRIPTOR_DTS(
   BMUXlib_TS_Legacy_Handle hMuxTS,
   const BMUXlib_Input_Descriptor *pstDescriptor
   );

uint32_t
BMUXLIB_TS_P_INPUT_DESCRIPTOR_ESCR(
   BMUXlib_TS_Legacy_Handle hMuxTS,
   const BMUXlib_Input_Descriptor *pstDescriptor
   );

#define BMUXLIB_P_LOCK_METADATA_MEMORY(pDesc, block) bmuxlib_p_lock_metadata_memory(pDesc, block, __FILE__, __LINE__)
void bmuxlib_p_lock_metadata_memory(BMUXlib_TS_P_TransportDescriptorMetaData *pDesc, BMMA_Block_Handle block, const char *file, int line);

#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_TS_PRIV_H_ */
