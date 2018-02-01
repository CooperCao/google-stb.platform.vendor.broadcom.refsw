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

#ifndef BVCE_PRIV_H_
#define BVCE_PRIV_H_

#include "bvce_fw_api_common.h"
#include "bvce_platform.h"
#include "bvce_telem.h"
#include "bkni_multi.h"
#include "bvce_debug_priv.h"
#include "bvce_buffer.h"
#include "bdbg_fifo.h"
#include "bvce_debug_common.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

BDBG_OBJECT_ID_DECLARE(BVCE_P_Output_Context);    /* BVCE_Output_Handle */
BDBG_OBJECT_ID_DECLARE(BVCE_P_Channel_Context);   /* BVCE_Channel_Handle */
BDBG_OBJECT_ID_DECLARE(BVCE_P_Context);           /* BVCE_Handle */

/* DEBUG */
#ifndef BVCE_P_DUMP_OUTPUT_CDB
#define BVCE_P_DUMP_OUTPUT_CDB 0
#endif
#ifndef BVCE_P_DUMP_OUTPUT_ITB
#define BVCE_P_DUMP_OUTPUT_ITB 0
#endif

/* Signatures */
#define BVCE_P_SIGNATURE_BASE                               0x56434500 /* "VCE" in ASCII */
#define BVCE_P_SIGNATURE_OPENSETTINGS                       (BVCE_P_SIGNATURE_BASE+1)
#define BVCE_P_SIGNATURE_OUTPUTOPENSETTINGS                 (BVCE_P_SIGNATURE_BASE+2)
#define BVCE_P_SIGNATURE_ALLOCBUFFERSSETTINGS               (BVCE_P_SIGNATURE_BASE+3)
#define BVCE_P_SIGNATURE_CHANNELOPENSETTINGS                (BVCE_P_SIGNATURE_BASE+4)
#define BVCE_P_SIGNATURE_CHANNELENCODESETTINGS              (BVCE_P_SIGNATURE_BASE+5)
#define BVCE_P_SIGNATURE_STARTENCODESETTINGS                (BVCE_P_SIGNATURE_BASE+6)
#define BVCE_P_SIGNATURE_DEVICECALLBACKSETTINGS             (BVCE_P_SIGNATURE_BASE+7)
#define BVCE_P_SIGNATURE_CHANNELCALLBACKSETTINGS            (BVCE_P_SIGNATURE_BASE+8)
#define BVCE_P_SIGNATURE_CHANNELENCODESETTINGSONINPUTCHANGE (BVCE_P_SIGNATURE_BASE+9)
#define BVCE_P_SIGNATURE_ENCODESETTINGS                     (BVCE_P_SIGNATURE_BASE+10)
#define BVCE_P_SIGNATURE_STOPENCODESETTINGS                 (BVCE_P_SIGNATURE_BASE+11)
#define BVCE_P_SIGNATURE_DUMPSTATESETTINGS                  (BVCE_P_SIGNATURE_BASE+12)

#define BVCE_P_DEFAULT_CDB_SIZE MIN_CDB_SIZE_IN_BYTES
#define BVCE_P_DEFAULT_CDB_ALIGNMENT 256
#define BVCE_P_DEFAULT_ITB_SIZE MIN_ITB_SIZE_IN_BYTES
#define BVCE_P_DEFAULT_ITB_ALIGNMENT 256

#define BVCE_P_DEFAULT_DEBUG_LOG_SIZE (512*1024)
#define BVCE_P_MAX_DEBUG_FIFO_COUNT 4096

#define BVCE_P_DEFAULT_USER_DATA_QUEUE_SIZE 32

#define BVCE_P_FIRMWARE_BOOT_TIMEOUT 1000
#define BVCE_P_FIRMWARE_COMMAND_TIMEOUT 1000

#define BVCE_P_MAX_VIDEODESCRIPTORS 3192

#define BVCE_P_MAX_METADATADESCRIPTORS 8

#define BVCE_P_DEFAULT_ALIGNMENT 1024

#define BVCE_P_ALIGN(x,bytes) ((((x) + (bytes-1))/(bytes))*(bytes))

#define BVCE_P_NEW_METADATA_MASK 0x0FFFFFFF

#define BVCE_P_SET_32BIT_HI_LO_FROM_64( _32bit, _64bit ) { (_32bit ## Hi) = (uint32_t)( ((_64bit) >> 32) & 0xFFFFFFFF ); (_32bit ## Lo) = (uint32_t)( ((_64bit) >> 0) & 0xFFFFFFFF ); }

typedef union BVCE_P_Command
{
      uint32_t data[HOST_CMD_BUFFER_SIZE / sizeof(uint32_t)];

      union {
            struct {
                  uint32_t uiCommand;
                  uint32_t uiChannel;
            } stGeneric;
            ViceCmdInit_t stInit;
            ViceCmdOpenChannel_t stOpenChannel;
            ViceCmdConfigChannel_t stConfigChannel;
            ViceCmdStartChannel_t stStartChannel;
            ViceCmdStopChannel_t stStopChannel;
            ViceCmdCloseChannel_t stCloseChannel;
            ViceCmdGetChannelStatus_t stGetChannelStatus;
            ViceCmdDebugChannel_t stDebugChannel;
      } type;
} BVCE_P_Command;

typedef union BVCE_P_Response
{
      uint32_t data[HOST_CMD_BUFFER_SIZE / sizeof(uint32_t)];

      union
      {
            struct {
                  uint32_t uiCommand;
                  uint32_t uiStatus;
            } stGeneric;
            ViceCmdInitResponse_t stInit;
            ViceCmdOpenChannelResponse_t stOpenChannel;
            ViceCmdGetChannelStatusResponse_t stGetChannelStatus;
      } type;
} BVCE_P_Response;

typedef enum BVCE_P_Status
{
   BVCE_P_Status_eIdle,
   BVCE_P_Status_eOpened,
   BVCE_P_Status_eStarted,
   BVCE_P_Status_eStopping,

   BVCE_P_Status_eMax
} BVCE_P_Status;

BVCE_P_Status
BVCE_Channel_P_GetState(
   BVCE_Channel_Handle hVceCh
   );

void
BVCE_Channel_P_GetStartEncodeSettings(
   BVCE_Channel_Handle hVceCh,
   BVCE_Channel_StartEncodeSettings *pstStartEncodeSettings
   );

void
BVCE_Channel_P_GetEncodeSettings(
   BVCE_Channel_Handle hVceCh,
   BVCE_Channel_EncodeSettings *pstEncodeSettings
   );

void
BVCE_Channel_P_HandleEOSEvent(
   BVCE_Channel_Handle hVceCh
   );

BERR_Code
BVCE_P_EnableInterrupts(
         BVCE_Handle hVce,
         bool bEnable
         );

typedef enum BVCE_P_HeapId
{
   BVCE_P_HeapId_eSystem,
   BVCE_P_HeapId_ePicture,
   BVCE_P_HeapId_eSecure,

   BVCE_P_HeapId_eMax
} BVCE_P_HeapId;

typedef struct BVCE_P_OutputBuffers
{
      BVCE_Output_AllocBuffersSettings stSettings;

      struct
      {
         BVCE_P_Allocator_Handle hAllocator;
         BVCE_P_Buffer_Handle hBuffer;
      } stITB, stCDB;
} BVCE_P_OutputBuffers;

typedef struct BVCE_P_Output_Cache
{
      bool bValid;
      BVCE_Channel_StartEncodeSettings stStartEncodeSettings;
      BVCE_Channel_EncodeSettings stEncodeSettings;
} BVCE_P_Output_Cache;

typedef struct BVCE_P_Output_BufferCache
{
      uint64_t uiITBCacheValidOffset;
      uint64_t uiCDBCacheValidOffset;
} BVCE_P_Output_BufferCache;

#define BVCE_P_ITBENTRY_ENTRYTYPE_OFFSET 0
#define BVCE_P_ITBENTRY_ENTRYTYPE_SHIFT 24
#define BVCE_P_ITBENTRY_ENTRYTYPE_MASK 0xFF

/* Base Entry */
#define BVCE_P_ITBENTRY_ERROR_OFFSET 0
#define BVCE_P_ITBENTRY_ERROR_SHIFT 23
#define BVCE_P_ITBENTRY_ERROR_MASK 0x00000001

#define BVCE_P_ITBENTRY_CDBADDRESS_OFFSET 1
#define BVCE_P_ITBENTRY_CDBADDRESS_SHIFT 0
#define BVCE_P_ITBENTRY_CDBADDRESS_MASK 0xFFFFFFFF

/* Timestamp Entry */
#define BVCE_P_ITBENTRY_DTSVALID_OFFSET 0
#define BVCE_P_ITBENTRY_DTSVALID_SHIFT 15
#define BVCE_P_ITBENTRY_DTSVALID_MASK 0x00000001

#define BVCE_P_ITBENTRY_PTS32_OFFSET 0
#define BVCE_P_ITBENTRY_PTS32_SHIFT 1
#define BVCE_P_ITBENTRY_PTS32_MASK 0x00000001

#define BVCE_P_ITBENTRY_DTS32_OFFSET 0
#define BVCE_P_ITBENTRY_DTS32_SHIFT 0
#define BVCE_P_ITBENTRY_DTS32_MASK 0x00000001

#define BVCE_P_ITBENTRY_PTS_OFFSET 1
#define BVCE_P_ITBENTRY_PTS_SHIFT 0
#define BVCE_P_ITBENTRY_PTS_MASK 0xFFFFFFFF

#define BVCE_P_ITBENTRY_DTS_OFFSET 2
#define BVCE_P_ITBENTRY_DTS_SHIFT 0
#define BVCE_P_ITBENTRY_DTS_MASK 0xFFFFFFFF

#define BVCE_P_ITBENTRY_IFRAME_OFFSET 3
#define BVCE_P_ITBENTRY_IFRAME_SHIFT 8
#define BVCE_P_ITBENTRY_IFRAME_MASK 0x00000001

#define BVCE_P_ITBENTRY_HDRONLY_OFFSET 3
#define BVCE_P_ITBENTRY_HDRONLY_SHIFT 9
#define BVCE_P_ITBENTRY_HDRONLY_MASK 0x00000001

/* Transmission Entry */
#define BVCE_P_ITBENTRY_SHR_OFFSET 1
#define BVCE_P_ITBENTRY_SHR_SHIFT 16
#define BVCE_P_ITBENTRY_SHR_MASK 0x0000FFFF

#define BVCE_P_ITBENTRY_TICKSPERBIT_OFFSET 1
#define BVCE_P_ITBENTRY_TICKSPERBIT_SHIFT 0
#define BVCE_P_ITBENTRY_TICKSPERBIT_MASK 0x0000FFFF

/* ESCR Entry */
#define BVCE_P_ITBENTRY_ESCR_OFFSET 1
#define BVCE_P_ITBENTRY_ESCR_SHIFT 0
#define BVCE_P_ITBENTRY_ESCR_MASK 0xFFFFFFFF

#define BVCE_P_ITBENTRY_OPTS_OFFSET 2
#define BVCE_P_ITBENTRY_OPTS_SHIFT 0
#define BVCE_P_ITBENTRY_OPTS_MASK 0xFFFFFFFF

#define BVCE_P_ITBENTRY_METADATA_OFFSET 3
#define BVCE_P_ITBENTRY_METADATA_SHIFT 0
#define BVCE_P_ITBENTRY_METADATA_MASK 0xFFFFFFFF

/* NAL Addr/Data Entry */
#define BVCE_P_ITBENTRY_NALADDR_STARTCODESIZE_OFFSET 0
#define BVCE_P_ITBENTRY_NALADDR_STARTCODESIZE_SHIFT 6
#define BVCE_P_ITBENTRY_NALADDR_STARTCODESIZE_MASK 0x7

#define BVCE_P_ITBENTRY_NALADDR_CDBOFFSET_OFFSET 1
#define BVCE_P_ITBENTRY_NALADDR_CDBOFFSET_SHIFT 0
#define BVCE_P_ITBENTRY_NALADDR_CDBOFFSET_MASK 0xFFFFFFFF

#define BVCE_P_ITBENTRY_NALADDR_DATA_OFFSET 2

/* NAL Data Entry */
#define BVCE_P_ITBENTRY_NALDATA_DATASTART_OFFSET 0
#define BVCE_P_ITBENTRY_NALDATA_DATASTART_SHIFT 0
#define BVCE_P_ITBENTRY_NALDATA_DATASTART_MASK 0x1

#define BVCE_P_ITBENTRY_NALDATA_DATAEND_OFFSET 0
#define BVCE_P_ITBENTRY_NALDATA_DATAEND_SHIFT 1
#define BVCE_P_ITBENTRY_NALDATA_DATAEND_MASK 0x1

#define BVCE_P_ITBENTRY_NALDATA_DATASIZE_OFFSET 0
#define BVCE_P_ITBENTRY_NALDATA_DATASIZE_SHIFT 2
#define BVCE_P_ITBENTRY_NALDATA_DATASIZE_MASK 0xF

#define BVCE_P_ITBENTRY_NALDATA_DATA_MAXOFFSET 3
#define BVCE_P_ITBENTRY_NALDATA_DATA_OFFSET 1

/* EOS Entry */
#define BVCE_P_ITBENTRY_EOS_CDBADDRESS_OFFSET 1
#define BVCE_P_ITBENTRY_EOS_CDBADDRESS_SHIFT 0
#define BVCE_P_ITBENTRY_EOS_CDBADDRESS_MASK 0xFFFFFFFF

#define BVCE_P_ITBENTRY_EOS_ESCR_OFFSET 2
#define BVCE_P_ITBENTRY_EOS_ESCR_SHIFT 0
#define BVCE_P_ITBENTRY_EOS_ESCR_MASK 0xFFFFFFFF

/* STC Entry */
#define BVCE_P_ITBENTRY_STCSNAPSHOT_UPPER_OFFSET 0
#define BVCE_P_ITBENTRY_STCSNAPSHOT_UPPER_SHIFT 0

#define BVCE_P_ITBENTRY_STCSNAPSHOT_UPPER_MASK 0x3FF

#define BVCE_P_ITBENTRY_STCSNAPSHOT_LOWER_OFFSET 1
#define BVCE_P_ITBENTRY_STCSNAPSHOT_LOWER_SHIFT 0
#define BVCE_P_ITBENTRY_STCSNAPSHOT_LOWER_MASK 0xFFFFFFFF

/* CRC Entry */
#define BVCE_P_ITBENTRY_CRCLOAD_OFFSET 0
#define BVCE_P_ITBENTRY_CRCLOAD_SHIFT 21
#define BVCE_P_ITBENTRY_CRCLOAD_MASK 0x00000001

#define BVCE_P_ITBENTRY_CRCCOMPARE_OFFSET 0
#define BVCE_P_ITBENTRY_CRCCOMPARE_SHIFT 20
#define BVCE_P_ITBENTRY_CRCCOMPARE_MASK 0x00000001

#define BVCE_P_ITBENTRY_CRCINIT_OFFSET 1
#define BVCE_P_ITBENTRY_CRCINIT_SHIFT 0
#define BVCE_P_ITBENTRY_CRCINIT_MASK 0xFFFFFFFF

#define BVCE_P_ITBENTRY_CRCREMAINDER_OFFSET 2
#define BVCE_P_ITBENTRY_CRCREMAINDER_SHIFT 0
#define BVCE_P_ITBENTRY_CRCREMAINDER_MASK 0xFFFFFFFF

#if BVCE_P_DUMP_MAU_PERFORMANCE_DATA
#define BVCE_P_ITBENTRY_MAUPERFOVERFLOW_OFFSET 0
#define BVCE_P_ITBENTRY_MAUPERFOVERFLOW_SHIFT 0
#define BVCE_P_ITBENTRY_MAUPERFOVERFLOW_MASK 0x3F

#define BVCE_P_ITBENTRY_MAUPERFDATA0_OFFSET 1
#define BVCE_P_ITBENTRY_MAUPERFDATA0_SHIFT 0
#define BVCE_P_ITBENTRY_MAUPERFDATA0_MASK 0xFFFF

#define BVCE_P_ITBENTRY_MAUPERFDATA1_OFFSET 1
#define BVCE_P_ITBENTRY_MAUPERFDATA1_SHIFT 16
#define BVCE_P_ITBENTRY_MAUPERFDATA1_MASK 0xFFFF

#define BVCE_P_ITBENTRY_MAUPERFDATA2_OFFSET 2
#define BVCE_P_ITBENTRY_MAUPERFDATA2_SHIFT 0
#define BVCE_P_ITBENTRY_MAUPERFDATA2_MASK 0xFFFF

#define BVCE_P_ITBENTRY_MAUPERFDATA3_OFFSET 2
#define BVCE_P_ITBENTRY_MAUPERFDATA3_SHIFT 16
#define BVCE_P_ITBENTRY_MAUPERFDATA3_MASK 0xFFFF

#define BVCE_P_ITBENTRY_MAUPERFDATA4_OFFSET 3
#define BVCE_P_ITBENTRY_MAUPERFDATA4_SHIFT 0
#define BVCE_P_ITBENTRY_MAUPERFDATA4_MASK 0xFFFF

#define BVCE_P_ITBENTRY_MAUPERFDATA5_OFFSET 3
#define BVCE_P_ITBENTRY_MAUPERFDATA5_SHIFT 16
#define BVCE_P_ITBENTRY_MAUPERFDATA5_MASK 0xFFFF
#endif

#define BVCE_P_ITBEntry_Get(_pentry, _field) ((((uint32_t*)(_pentry))[BVCE_P_ITBENTRY_##_field##_OFFSET] >> BVCE_P_ITBENTRY_##_field##_SHIFT ) & BVCE_P_ITBENTRY_##_field##_MASK )

#define BVCE_P_ITBEntry_GetEntryType(_pentry) BVCE_P_ITBEntry_Get(_pentry, ENTRYTYPE)
#define BVCE_P_ITBEntry_GetCDBAddress(_pentry) BVCE_P_ITBEntry_Get(_pentry, CDBADDRESS)
#define BVCE_P_ITBEntry_GetError(_pentry) BVCE_P_ITBEntry_Get(_pentry, ERROR)
#define BVCE_P_ITBEntry_GetDTS(_pentry) ( ( ( (uint64_t) BVCE_P_ITBEntry_Get(_pentry, DTS32)) << 32 ) | BVCE_P_ITBEntry_Get(_pentry, DTS) )
#define BVCE_P_ITBEntry_GetDTSValid(_pentry) BVCE_P_ITBEntry_Get(_pentry, DTSVALID)
#define BVCE_P_ITBEntry_GetPTS(_pentry) ( ( ( (uint64_t) BVCE_P_ITBEntry_Get(_pentry, PTS32)) << 32 ) | BVCE_P_ITBEntry_Get(_pentry, PTS) )
#define BVCE_P_ITBEntry_GetIFrame(_pentry) BVCE_P_ITBEntry_Get(_pentry, IFRAME)
#define BVCE_P_ITBEntry_GetHDROnly(_pentry) BVCE_P_ITBEntry_Get(_pentry, HDRONLY)
#define BVCE_P_ITBEntry_GetTicksPerBit(_pentry) ((uint16_t) BVCE_P_ITBEntry_Get(_pentry, TICKSPERBIT))
#define BVCE_P_ITBEntry_GetSHR(_pentry) ((int16_t) BVCE_P_ITBEntry_Get(_pentry, SHR))
#define BVCE_P_ITBEntry_GetESCR(_pentry) BVCE_P_ITBEntry_Get(_pentry, ESCR)
#define BVCE_P_ITBEntry_GetOriginalPTS(_pentry) BVCE_P_ITBEntry_Get(_pentry, OPTS)
#define BVCE_P_ITBEntry_GetMetadata(_pentry) BVCE_P_ITBEntry_Get(_pentry, METADATA)
#define BVCE_P_ITBEntry_GetNalAddr_CDBOffset(_pentry) BVCE_P_ITBEntry_Get(_pentry, NALADDR_CDBOFFSET)
#define BVCE_P_ITBEntry_GetNalAddr_NumStartBytes(_pentry) BVCE_P_ITBEntry_Get(_pentry, NALADDR_STARTCODESIZE)
#define BVCE_P_ITBEntry_GetNalData_NumDataBytes(_pentry) BVCE_P_ITBEntry_Get(_pentry, NALDATA_DATASIZE)
#define BVCE_P_ITBEntry_GetNalData_NalDataEnd(_pentry) BVCE_P_ITBEntry_Get(_pentry, NALDATA_DATAEND)
#define BVCE_P_ITBEntry_GetNalData_NalDataStart(_pentry) BVCE_P_ITBEntry_Get(_pentry, NALDATA_DATASTART)
#define BVCE_P_ITBEntry_GetEOSCDBAddress(_pentry) BVCE_P_ITBEntry_Get(_pentry, EOS_CDBADDRESS)
#define BVCE_P_ITBEntry_GetEOSESCR(_pentry) BVCE_P_ITBEntry_Get(_pentry, EOS_ESCR)
#define BVCE_P_ITBEntry_GetStcSnapshot(_pentry) ( ( ( (uint64_t) BVCE_P_ITBEntry_Get(_pentry, STCSNAPSHOT_UPPER)) << 32 ) | BVCE_P_ITBEntry_Get(_pentry, STCSNAPSHOT_LOWER) )
#define BVCE_P_ITBEntry_GetCRCLoad(_pentry) BVCE_P_ITBEntry_Get(_pentry, CRCLOAD)
#define BVCE_P_ITBEntry_GetCRCCompare(_pentry) BVCE_P_ITBEntry_Get(_pentry, CRCCOMPARE)
#define BVCE_P_ITBEntry_GetCRCInit(_pentry) BVCE_P_ITBEntry_Get(_pentry, CRCINIT)
#define BVCE_P_ITBEntry_GetCRCRemainder(_pentry) BVCE_P_ITBEntry_Get(_pentry, CRCREMAINDER)
#if BVCE_P_DUMP_MAU_PERFORMANCE_DATA
#define BVCE_P_ITBEntry_GetMAUPerformanceOverflowStatus(_pentry) BVCE_P_ITBEntry_Get(_pentry, MAUPERFOVERFLOW)
#define BVCE_P_ITBEntry_GetMAUPerformanceData(_pentry,_index) BVCE_P_ITBEntry_Get(_pentry, MAUPERFDATA##_index)
#endif

typedef struct BVCE_P_Output_ITB_IndexEntry
{
   unsigned uiSizeInITB;
   bool bError;
   uint64_t uiCDBAddress;
   BAVC_VideoBufferDescriptor stFrameDescriptor;
   uint32_t uiMetadata;
   bool bIgnoreFrame;
   bool bChannelChange;
   unsigned uiChunkId;
   uint8_t uiFrameRate;
   uint16_t uiWidth;
   uint16_t uiHeight;
   uint32_t uiSTC;
   bool bHeaderOnly;
   bool bForceHeaderOnly;
#if BVCE_P_DUMP_MAU_PERFORMANCE_DATA
   struct
   {
      bool bValid;
      bool bOverflow;
      uint16_t uiValue;
   } stMAUPerformanceData[24];
#endif
} BVCE_P_Output_ITB_IndexEntry;

typedef enum BVCE_P_Output_BufferAccessMode
{
   BVCE_P_Output_BufferAccessMode_eUnknown = 0, /* eUnknown if BVCE_Output_GetRegisters() and BVCE_Output_ConsumeBufferDescriptors() weren't called */
   BVCE_P_Output_BufferAccessMode_eDirect = 1,  /* eDirect if BVCE_Output_GetRegisters() was called and BVCE_Output_ConsumeBufferDescriptors() was never called */
   BVCE_P_Output_BufferAccessMode_eDescriptor   /* eDescriptor if BVCE_Output_ConsumeBufferDescriptors() was called */
} BVCE_P_Output_BufferAccessMode;

typedef enum BVCE_Output_P_DataUnitDetectState
{
   BVCE_Output_P_DataUnitDetectState_eLookForNALU = 0,
   BVCE_Output_P_DataUnitDetectState_eSkipToNextFrameStart,

   BVCE_Output_P_DataUnitDetectState_eMax
} BVCE_Output_P_DataUnitDetectState;

typedef struct BVCE_P_Buffer_Offsets
{
   uint64_t uiBase;
   uint64_t uiEnd;
   uint64_t uiValid;
   uint64_t uiRead;
   uint64_t uiWrite;
   uint64_t uiShadowRead;
} BVCE_P_Buffer_Offsets;

typedef struct BVCE_P_VideoBufferDescriptorReleaseMetadata
{
   unsigned uiShadowLength;
   bool bHasITBEntry;
} BVCE_P_VideoBufferDescriptorReleaseMetadata;

typedef struct BVCE_P_Output_Context
{
      BDBG_OBJECT(BVCE_P_Output_Context)

      BVCE_Handle hVce;
      BVCE_P_Status eStatus;
      BVCE_Output_OpenSettings stOpenSettings;
      BAVC_VideoContextMap stRegisters;
      BVCE_OutputBuffers_Handle hOutputBuffers;
      struct
      {
         BVCE_P_Allocator_Handle hAllocator;
         BVCE_P_Buffer_Handle hDescriptorBuffer;
         BAVC_VideoBufferDescriptor *astDescriptors;
         BVCE_P_VideoBufferDescriptorReleaseMetadata *astDescriptorReleaseMetadata;
         BVCE_P_Buffer_Handle hMetadataBuffer;
         BAVC_VideoMetadataDescriptor *astMetadataDescriptors;
         BVCE_P_Buffer_Handle hITBIndexBuffer;
         BVCE_P_Output_ITB_IndexEntry *astIndex;
      } stDescriptors;


      struct
      {
         BVCE_Channel_Handle hVceCh;
         bool bCabacInitializedActual;
         bool bCabacInitializedShadow;

         struct
         {
            uint64_t uiShadowDepth;

            BVCE_P_Buffer_Offsets stOffset;

            uint32_t uiIndexWriteOffset;
            uint32_t uiIndexReadOffset;
            uint32_t uiIndexShadowReadOffset;

            bool bReadHackDone;

            uint64_t uiPreviousValidOffset;
            bool bPreviousValidOffsetValid;
         } stITBBuffer;

         struct
         {
            uint64_t uiShadowValidOffset;

            BVCE_P_Buffer_Offsets stOffset;

#if BVCE_P_DUMP_OUTPUT_ITB_DESC
            struct
            {
               uint64_t uiValidOffset;
               uint64_t uiReadOffset;
               uint64_t uiWriteOffset;
               uint64_t uiDepth;
            } hw;
#endif
            uint64_t uiPreviousValidOffset;
            bool bPreviousValidOffsetValid;
         } stCDBBuffer;

         struct
         {
            BVCE_Output_P_DataUnitDetectState eState;
            bool bFound;
            uint8_t uiType;
         } stDataUnitDetect;

         uint32_t uiDescriptorWriteOffset;
         uint32_t uiDescriptorReadOffset;

         uint32_t uiMetadataDescriptorWriteOffset;
         uint32_t uiMetadataDescriptorReadOffset;

         /* 7425A0: The following is a hack to prevent HW/SW from getting confused about CDB buffer
          * empty vs fullness.  We always offset the ITB/CDB read pointers by 1 less than what was
          * actually read so that the HW thinks there's always at least 1 byte in the CDB buffer.
          * This essentially changes the meaning of the READ pointer from "the byte TO BE read NEXT"
          * to "the bytes THAT WAS read LAST"
          */
         bool bCDBReadHackDone;
         bool bEOSITBEntryIndexed;
         bool bEOSDescriptorSent;
         /* 7425A0: END HACK */

         bool bNewITBEntry;

         BVCE_P_Output_Cache stChannelCache;
         uint32_t uiDataUnitDetectionShiftRegister;

         BVCE_P_Output_BufferCache stBufferCache;

         bool bMetadataSent;
         unsigned uiPreviousMetadata;

         unsigned uiPendingDescriptors;
         unsigned uiConsumedDescriptors;

         BVCE_P_Output_BufferAccessMode eBufferAccessMode;

         bool bPreviousESCRValid;
         uint32_t uiPreviousESCR;

         bool bPreviousChunkIdValid;
         unsigned uiPreviousChunkId;

         bool bChannelStatusValid;
         BVCE_Channel_Status stChannelStatus;

         bool bReadOffsetInitialized;
         bool bFirstFrameSeen;

         struct
         {
            uint64_t uiShadowRead;
            BAVC_VideoBufferDescriptor stVideoDescriptor;
            bool bPartiallyFilled;

            uint32_t uiPreviousDTS;
            bool bPreviousDTSValid;
         } stReadIndex;

         BAVC_VideoExtractedNALUInfo stExtractedNALUInfo;
         bool bExtractedNaluInfoDone;
      } state;

#if BVCE_P_DUMP_OUTPUT_CDB
      BVCE_Debug_Log_Handle hCDBDumpFile;
#endif
#if BVCE_P_DUMP_OUTPUT_ITB
      BVCE_Debug_Log_Handle hITBDumpFile;
#endif
#if BVCE_P_DUMP_OUTPUT_ITB_DESC
      BVCE_Debug_Log_Handle hITBDescDumpFile;
#endif
#if BVCE_P_DUMP_MAU_PERFORMANCE_DATA
      BVCE_Debug_Log_Handle hMAUPerformanceDumpFile;
#endif
#if BVCE_P_TEST_MODE
      BVCE_Debug_Log_Handle hDescriptorLog;
      unsigned uiDescriptorCount;
#endif
} BVCE_P_Output_Context;

typedef enum BVCE_P_PictureBufferType
{
   BVCE_P_PictureBufferType_eLuma,
   BVCE_P_PictureBufferType_eChroma,
   BVCE_P_PictureBufferType_e1VLuma,
   BVCE_P_PictureBufferType_e2VLuma,
   BVCE_P_PictureBufferType_eShiftedChroma,

   BVCE_P_PictureBufferType_eMax
} BVCE_P_PictureBufferType;

#define BVCE_P_PICTURE_OFFSET_LUT_SIZE BVCE_P_FW_PICTURE_QUEUE_LENGTH
#define BVCE_P_DROP_QUEUE_SIZE BVCE_P_FW_PICTURE_QUEUE_LENGTH

typedef struct BVCE_P_PictureBlockInfo
{
   BMMA_Block_Handle hBlock;
   unsigned uiOffset;
   unsigned uiNMBY;
   bool bDcx;
   unsigned uiHorizontalDecimationShift;
   BSTD_DeviceOffset uiBlockOffset;
   BSTD_DeviceOffset uiPhysicalOffset;
} BVCE_P_PictureBlockInfo;

typedef struct BVCE_P_Channel_Context
{
      BDBG_OBJECT(BVCE_P_Channel_Context)

      BVCE_Handle hVce;
      BVCE_P_Status eStatus;
      BVCE_Channel_OpenSettings stOpenSettings;

      struct
      {
         BVCE_P_Allocator_Handle hAllocator;
         BVCE_P_Buffer_Handle hBuffer;
      } memory[BVCE_P_HeapId_eMax];

      struct {
         BVCE_P_Buffer_Handle hBuffer;
         struct {
               uint32_t uiUserDataQueueInfoAddress;
         } dccm;
         unsigned uiQueuedBuffers;
#if BVCE_P_DUMP_USERDATA_LOG
         BVCE_Debug_Log_Handle hUserDataLog;
#endif
         BVCE_FW_P_UserData_Queue stUserDataQueue; /* Temporary Copy */
      } userdata;

      struct {
         struct {
            uint32_t auiPictureReleaseQueueInfoAddress[BVCE_P_PictureBufferType_eMax];
         } dccm;

         struct
         {
            struct
            {
               struct
               {
                  bool bInUse;
                  BVCE_P_PictureBufferType ePictureBufferType;
                  BVCE_P_PictureBlockInfo blockInfo;
               } astEntry[BVCE_P_PICTURE_OFFSET_LUT_SIZE];
               unsigned uiNumEntries;
            } astLUT[BVCE_P_PictureBufferType_eMax];

            BVCE_P_FW_PictureBufferMailbox stPictureBufferMailbox; /* Temporary Copy */
            BVCE_P_FW_PictureBuffer_Queue stPictureBufferQueue; /* Temporary Copy */

            struct
            {
               unsigned uiReadOffset;
               unsigned uiWriteOffset;
               BAVC_EncodePictureBuffer astPicture[BVCE_P_DROP_QUEUE_SIZE];
            } dropQueue;

            bool bLastOriginalPTSValid;
            uint32_t ulLastOriginalPTS;

            uint64_t uiNextNewPTSIn360Khz; /* PTS of incoming pictures for NRT mode */
            uint64_t uiNextSTCIn360Khz; /* STC for NRT mode */

            bool bNextTargetPTSin360KhzValid;
            uint64_t uiNextTargetPTSin360Khz; /* PTS of next picture to encode */

            bool bPreviousPictureIdValid;
            uint32_t uiPreviousPictureId;
            uint32_t uiPreviousSTCSnapshotLo;

            bool bPreviousEnqueueSTCSnapshotLoValid;
            uint32_t uiPreviousEnqueueSTCSnapshotLo;

            bool bPreviousPolarityReceivedValid;
            BAVC_Polarity ePreviousPolarityReceived;

            bool bPreviousPolarityEncodedValid;
            BAVC_Polarity ePreviousPolarityEncoded;

            struct
            {
               unsigned uiNumReceived;
               unsigned uiNumSentToVice;
               unsigned uiNumDroppedDueToFRC;
               unsigned uiNumDroppedDueToError;
            } stats;
         } stState;
      } picture;

      BVCE_Channel_StartEncodeSettings stStartEncodeSettings;
      BVCE_Channel_EncodeSettings stEncodeSettings;
      BVCE_Channel_EncodeSettings stPendingEncodeSettings;
      bool bPendingEncodeSettings;
      BVCE_Channel_CallbackSettings stCallbackSettings;
      BVCE_Channel_StopEncodeSettings stStopEncodeSettings;

      BVCE_Channel_Status stStatus;

      struct
      {
         BVCE_OutputBuffers_Handle hVceOutputBuffers;
         BVCE_Output_Handle hVceOutput;
      } stOutput;

#if BVCE_P_TEST_MODE
      BVCE_Debug_Log_Handle hConfigLog;
      BVCE_Debug_Log_Handle hStatusLog;
#endif
} BVCE_P_Channel_Context;

#define BVCE_P_CALLBACK_WATCHDOG 0
#define BVCE_P_CALLBACK_MAILBOX (BVCE_P_CALLBACK_WATCHDOG + BVCE_PLATFORM_P_NUM_ARC_CORES)
#define BVCE_P_CALLBACK_EVENT ( BVCE_P_CALLBACK_MAILBOX + 1 )
#define BVCE_P_CALLBACK_DATAREADY ( BVCE_P_CALLBACK_EVENT + 1)
#define BVCE_P_CALLBACK_MAX ( BVCE_P_CALLBACK_DATAREADY + BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS )

typedef struct BVCE_P_Context
{
      BDBG_OBJECT(BVCE_P_Context)

      BVCE_OpenSettings stOpenSettings;

      BTMR_TimerHandle hTimer;

      struct
      {
            BCHP_Handle hChp;
            BREG_Handle hReg;
            BINT_Handle hInt;
            BMMA_Heap_Handle hMem;
      } handles;

      struct
      {
            struct
            {
               BVCE_P_Allocator_Handle hAllocator;
               BVCE_P_Buffer_Handle hBuffer;
            } memory[BVCE_PLATFORM_P_NUM_ARC_CORES];

            BAFL_FirmwareLoadInfo astFirmwareLoadInfo[BVCE_PLATFORM_P_NUM_ARC_CORES];

            struct
            {
                  uint32_t uiRegisterBaseAddress[BVCE_PLATFORM_P_NUM_ARC_CORES];
                  uint32_t uiCommandBufferBaseOffset;
                  uint32_t uiCommandBufferBaseAddress;
                  uint32_t uiChannelErrorStatusBaseAddress;
                  uint32_t uiWatchdogErrorCodeBaseAddress[BVCE_PLATFORM_P_NUM_ARC_CORES];
            } dccm;

            struct
            {
                  uint32_t uiBufferInfoBaseAddress;
                  BVCE_Telem_Handle hVceTelem;
                  BVCE_P_Buffer_Handle hBuffer;
            } debug[BVCE_PLATFORM_P_NUM_ARC_CORES];

            BVCE_P_Command stCommand;
            BVCE_P_Response stResponse;
      } fw;

      struct
      {
            BINT_CallbackHandle ahCallbacks[BVCE_P_CALLBACK_MAX];
            BVCE_CallbackSettings stCallbackSettings;
      } callbacks;

      struct
      {
            BKNI_EventHandle hMailbox;
      } events;

      BVCE_P_Allocator_Handle ahAllocator[BVCE_P_HeapId_eMax];

      struct
      {
            BVCE_P_Output_Context context;
      } outputs[BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS];

      struct
      {
            BVCE_P_Channel_Context context;
            bool bResume;
      } channels[BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS];

#if ( BVCE_P_CORE_MAJOR <= 2 )
      BVCE_P_Buffer_Handle hCabacCmdBuffer;
#endif

      BVCE_Platform_P_Config stPlatformConfig;

#if BVCE_P_DUMP_ARC_DEBUG_LOG
      BVCE_Debug_Log_Handle hDebugLogDumpFile[BVCE_PLATFORM_P_NUM_ARC_CORES];
#endif

      BVCE_VersionInfo stVersionInfo;

      bool bBooted;
      bool bWatchdogOccurred;

      struct
      {
         unsigned uiElementSize;
         unsigned uiBufferSize;
         BVCE_P_Buffer_Handle hBuffer;

         void *pBuffer;
         BDBG_Fifo_Handle hDebugFifo;
      } stDebugFifo;
} BVCE_P_Context;

typedef struct BVCE_P_SendCommand_ConfigChannel_Settings
{
   bool bOnInputChange;
   bool bBeginNewRAP;
   bool bFastChannelChange;
} BVCE_P_SendCommand_ConfigChannel_Settings;

typedef struct BVCE_DebugFifo_EntryMetadata
{
   BVCE_DebugFifo_EntryType eType;
   unsigned uiInstance;
   unsigned uiChannel;
   unsigned uiTimestamp; /* (in milliseconds) */
} BVCE_DebugFifo_EntryMetadata;

#define BVCE_P_FUNCTION_TRACE_LENGTH 64
#define BVCE_P_FUNCTION_TRACE_ENTER(_level, _hVce, _uiChannel) BVCE_P_FUNCTION_TRACE(_level, "Enter:", _hVce, _uiChannel);
#define BVCE_P_FUNCTION_TRACE_LEAVE(_level, _hVce, _uiChannel) BVCE_P_FUNCTION_TRACE(_level, "Leave:", _hVce, _uiChannel);
#define BVCE_P_FUNCTION_TRACE_ENTER_isr(_level, _hVce, _uiChannel) BVCE_P_FUNCTION_TRACE_isr(_level, "Enter:", _hVce, _uiChannel);
#define BVCE_P_FUNCTION_TRACE_LEAVE_isr(_level, _hVce, _uiChannel) BVCE_P_FUNCTION_TRACE_isr(_level, "Leave:", _hVce, _uiChannel);
#define BVCE_P_FUNCTION_TRACE(_level, _szPrefix, _hVce, _uiChannel) \
if ( NULL != (_hVce)->stDebugFifo.hDebugFifo )\
{\
   BVCE_P_DebugFifo_Entry *pstEntry;\
   BDBG_Fifo_Token stToken;\
\
   pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( (_hVce)->stDebugFifo.hDebugFifo, &stToken );\
   if ( NULL != pstEntry )\
   {\
      pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eTrace##_level;\
      pstEntry->stMetadata.uiInstance = ((BVCE_Handle)(_hVce))->stOpenSettings.uiInstance;\
      pstEntry->stMetadata.uiChannel = _uiChannel;\
      pstEntry->stMetadata.uiTimestamp = 0;\
      ( NULL != (_hVce)->hTimer ) ? BTMR_ReadTimer( (_hVce)->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;\
      BKNI_Snprintf(pstEntry->data.szFunctionTrace, BVCE_P_FUNCTION_TRACE_LENGTH, "%s%s",_szPrefix,BSTD_FUNCTION);\
      BDBG_Fifo_CommitBuffer( &stToken );\
   }\
}

#define BVCE_P_FUNCTION_TRACE_isr(_level, _szPrefix, _hVce, _uiChannel) \
if ( NULL != (_hVce)->stDebugFifo.hDebugFifo )\
{\
   BVCE_P_DebugFifo_Entry *pstEntry;\
   BDBG_Fifo_Token stToken;\
\
   pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer_isrsafe( (_hVce)->stDebugFifo.hDebugFifo, &stToken );\
   if ( NULL != pstEntry )\
   {\
      pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eTrace##_level;\
      pstEntry->stMetadata.uiInstance = ((BVCE_Handle)(_hVce))->stOpenSettings.uiInstance;\
      pstEntry->stMetadata.uiChannel = _uiChannel;\
      pstEntry->stMetadata.uiTimestamp = 0;\
      ( NULL != (_hVce)->hTimer ) ? BTMR_ReadTimer_isr( (_hVce)->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;\
      BKNI_Snprintf(pstEntry->data.szFunctionTrace, BVCE_P_FUNCTION_TRACE_LENGTH, "%s%s",_szPrefix,BSTD_FUNCTION);\
      BDBG_Fifo_CommitBuffer_isrsafe( &stToken );\
   }\
}

typedef struct BVCE_P_DebugFifo_VideoExtractedNALUEntries
{
   unsigned uiNumValidBytes;
} BVCE_P_DebugFifo_VideoExtractedNALUEntry;

typedef struct BVCE_P_DebugFifo_VideoExtractedNALUInfo
{
   BVCE_P_DebugFifo_VideoExtractedNALUEntry astExtractedNALUEntry[BAVC_VIDEO_MAX_EXTRACTED_NALU_COUNT];
   unsigned uiNumExtractedNaluEntries;
} BVCE_P_DebugFifo_VideoExtractedNALUInfo;

typedef struct BVCE_P_DebugFifo_VideoMetadataDescriptor
{
      uint32_t uiMetadataFlags;

      BAVC_VideoBufferInfo stBufferInfo;

      struct
      {
            uint32_t uiMax; /* in bits/sec */
      } stBitrate;

      struct
      {
            BAVC_FrameRateCode eFrameRateCode;
      } stFrameRate;

      struct
      {
            struct
            {
                  uint16_t uiWidth;
                  uint16_t uiHeight;
            } coded;
      } stDimension;

      union
      {
            BAVC_VideoMetadata_VC1 stVC1; /* Applies for BAVC_VideoCompressionStd_eVC1SimpleMain */
      } uProtocolData;

      struct
      {
            uint64_t uiSTCSnapshot; /* Initial 42-bit STC Snapshot from video encode */
            unsigned uiChunkId; /* The FNRT chunk ID for the subsequent frame descriptors */
            unsigned uiEtsDtsOffset; /* The ETS to DTS offset for the encode session as determined by RC */
      } stTiming;

      BVCE_P_DebugFifo_VideoExtractedNALUInfo stExtractedNALUInfo; /* If the video encoder ITB contains NALU data beyond the NAL unit header,
                                                                    * then ALL of the NALU bytes are copied to this data structure. Each array will
                                                                    * start with the NAL unit header. (1 byte for AVC, 2 bytes for HEVC, etc) */
} BVCE_P_DebugFifo_VideoMetadataDescriptor;

#define BVCE_P_DEBUG_ENTRY(_hVce, _entryType, _uiChannel, _struct) \
{\
   BVCE_P_DebugFifo_Entry *pstEntry;\
   BDBG_Fifo_Token stToken;\
\
   pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( (_hVce)->stDebugFifo.hDebugFifo, &stToken );\
\
   if ( NULL != pstEntry )\
   {\
      pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_##_entryType;\
      pstEntry->stMetadata.uiInstance = (_hVce)->stOpenSettings.uiInstance;\
      pstEntry->stMetadata.uiChannel = (_uiChannel);\
      pstEntry->stMetadata.uiTimestamp = 0;\
      ( NULL != (_hVce)->hTimer ) ? BTMR_ReadTimer( (_hVce)->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;\
      BKNI_Memcpy( &pstEntry->data.stCommand, &(_struct), sizeof(_struct) );\
      BDBG_Fifo_CommitBuffer( &stToken );\
   }\
}

#define BVCE_P_DEBUG_ENTRY_isr(_hVce, _entryType, _uiChannel, _struct) \
{\
   BVCE_P_DebugFifo_Entry *pstEntry;\
   BDBG_Fifo_Token stToken;\
\
   pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( (_hVce)->stDebugFifo.hDebugFifo, &stToken );\
\
   if ( NULL != pstEntry )\
   {\
      pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_##_entryType;\
      pstEntry->stMetadata.uiInstance = (_hVce)->stOpenSettings.uiInstance;\
      pstEntry->stMetadata.uiChannel = (_uiChannel);\
      pstEntry->stMetadata.uiTimestamp = 0;\
      ( NULL != (_hVce)->hTimer ) ? BTMR_ReadTimer_isr( (_hVce)->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;\
      BKNI_Memcpy( &pstEntry->data.stCommand, &(_struct), sizeof(_struct) );\
      BDBG_Fifo_CommitBuffer( &stToken );\
   }\
}

typedef struct BVCE_P_DebugFifo_Entry
{
   BVCE_DebugFifo_EntryMetadata stMetadata;

   union
   {
      struct
      {
         BVCE_Channel_StartEncodeSettings stStartEncodeSettings;
         BVCE_Channel_EncodeSettings stEncodeSettings;
         BVCE_P_SendCommand_ConfigChannel_Settings stSettingsModifiers;
      } stConfig;
      BVCE_Channel_Status stStatus;
      BVCE_P_Output_ITB_IndexEntry stITBDescriptor;
      BAVC_VideoBufferDescriptor stBufferDescriptor;
      BVCE_P_DebugFifo_VideoMetadataDescriptor stMetadataDescriptor;
      struct
      {
         uint64_t uiReadOffset;
         uint8_t auiEntry[16];
      } stITB;
      BVCE_P_Command stCommand;
      BVCE_P_Response stResponse;
      char szFunctionTrace[BVCE_P_FUNCTION_TRACE_LENGTH];
      BVCE_P_Buffer_Offsets stOffset;
      BVCE_P_FW_PictureBufferMailbox stPictureBufferMailbox;
      BAVC_EncodePictureBuffer stPicture;
   } data;
} BVCE_P_DebugFifo_Entry;

extern const BAVC_FrameRateCode BVCE_P_FW2PI_FrameRateLUT[];
extern const unsigned BAVC_FrameRateCodeNumEntries;
extern const unsigned BAVC_FWMaxFrameRateCode;

void BVCE_P_ValidateStructSizes(void);
void BVCE_P_ValidateFrameRateEnum(void);

#ifdef __cplusplus
}
#endif

#endif /* BVCE_PRIV_H_ */
