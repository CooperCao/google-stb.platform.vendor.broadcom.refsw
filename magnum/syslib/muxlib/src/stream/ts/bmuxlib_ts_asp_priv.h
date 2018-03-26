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

#ifndef MAGNUM_SYSLIB_MUXLIB_SRC_STREAM_TS_BMUXLIB_TS_ASP_PRIV_H_
#define MAGNUM_SYSLIB_MUXLIB_SRC_STREAM_TS_BMUXLIB_TS_ASP_PRIV_H_

#include "bmuxlib_asp_ts_channel_context.h"
#include "bmuxlib_asp_ts_channel_priv.h"
#include "bmuxlib_asp_ts_source_sys_priv.h"
#include "bmuxlib_ts_consts.h"
#include "bmuxlib_ts.h"


#if BMUXLIB_TS_ASP_P_HAS_ASP
#include "basp_mux_fw_api.h"
#endif

#include "bmuxlib_ts_asp_interface.h"

/**********/
/* Handle */
/**********/
typedef struct BMUXlib_TS_ASP_P_Context *BMUXlib_TS_ASP_Handle;



/**************/
/* Prototypes */
/**************/
void
BMUXlib_TS_ASP_GetDefaultCreateSettings(
         BMUXlib_TS_CreateSettings *pCreateSettings
         );

BERR_Code
BMUXlib_TS_ASP_Create(
         BMUXlib_TS_ASP_Handle *phMuxTS,  /* [out] TSMuxer handle returned */
         const BMUXlib_TS_CreateSettings *pstCreateSettings
         );

BERR_Code
BMUXlib_TS_ASP_Destroy(
         BMUXlib_TS_ASP_Handle hMuxTS
         );

/****************/
/* Mux Settings */
/****************/
BERR_Code
BMUXlib_TS_ASP_SetMuxSettings(
         BMUXlib_TS_ASP_Handle hMuxTS,
         const BMUXlib_TS_MuxSettings *pstMuxSettings
         );

BERR_Code
BMUXlib_TS_ASP_GetMuxSettings(
         BMUXlib_TS_ASP_Handle hMuxTS,
         BMUXlib_TS_MuxSettings *pstMuxSettings
         );

/**************/
/* Start/Stop */
/**************/
/* BMUXlib_TS_Start - Configures the mux HW */
BERR_Code
BMUXlib_TS_ASP_Start(
         BMUXlib_TS_ASP_Handle hMuxTS,
         const BMUXlib_TS_StartSettings *pstStartSettings
         );

BERR_Code
BMUXlib_TS_ASP_Finish(
         BMUXlib_TS_ASP_Handle hMuxTS,
         const BMUXlib_TS_FinishSettings *pstFinishSettings
         );

BERR_Code
BMUXlib_TS_ASP_Stop(
         BMUXlib_TS_ASP_Handle hMuxTS
         );

/**********/
/* Memory */
/**********/
void
BMUXlib_TS_ASP_GetMemoryConfig(
         const BMUXlib_TS_MuxConfig *pstMuxConfig,
         BMUXlib_TS_MemoryConfig *pstMemoryConfig
         );

/***************/
/* System Data */
/***************/
BERR_Code
BMUXlib_TS_ASP_AddSystemDataBuffers(
         BMUXlib_TS_ASP_Handle hMuxTS,
         const BMUXlib_TS_SystemData *astSystemDataBuffer, /* Array of system data buffers */
         size_t uiCount, /* Count of system data buffers in array */
         size_t *puiQueuedCount /* Count of system data buffers queued by muxer (*puiQueuedCount <= uiCount) */
         );

BERR_Code
BMUXlib_TS_ASP_GetCompletedSystemDataBuffers(
         BMUXlib_TS_ASP_Handle hMuxTS,
         size_t *puiCompletedCount /* Returns count of system data buffers fully muxed */
         );

void
BMUXlib_TS_ASP_GetStatus(
   BMUXlib_TS_ASP_Handle hMuxTS,
   BMUXlib_TS_Status *pstStatus
   );

/***********/
/* Execute */
/***********/
BERR_Code
BMUXlib_TS_ASP_DoMux(
   BMUXlib_TS_ASP_Handle hMuxTS,
   BMUXlib_DoMux_Status *pstStatus
   );

typedef enum BMUXlib_TS_MemoryBlock_Type
{
   BMUXlib_TS_MemoryBlock_Type_eTSBuffer,
   BMUXlib_TS_MemoryBlock_Type_eUserdataInterface,
   BMUXlib_TS_MemoryBlock_Type_eAudioIndex,
   BMUXlib_TS_MemoryBlock_Type_eVideoIndex = BMUXlib_TS_MemoryBlock_Type_eAudioIndex + BMUXLIB_TS_MAX_AUDIO_PIDS,

   BMUXlib_TS_MemoryBlock_Type_eMax = BMUXlib_TS_MemoryBlock_Type_eVideoIndex + BMUXLIB_TS_MAX_VIDEO_PIDS
} BMUXlib_TS_MemoryBlock_Type;

#define BMUXLIB_TS_MAX_SYSTEM_DATA_COUNT BMUXLIB_ASP_TS_MAX_SYS_PACKETS_QUEUED

typedef struct BMUXlib_TS_ASP_P_MemoryBlock
{
   BMMA_Block_Handle hBlock;
   BMMA_DeviceOffset uiOffset;
   void* pBuffer;
} BMUXlib_TS_ASP_P_MemoryBlock;

#define BMUXLIB_TS_MAX_VIDEO_FRAME_COUNT 32

typedef struct BMUXlib_TS_ASP_P_SystemData_Entry
{
   BMUXlib_TS_SystemData stData;
   BMMA_DeviceOffset uiOffset;
} BMUXlib_TS_ASP_P_SystemData_Entry;

typedef struct BMUXlib_TS_ASP_P_Context
{
   BDBG_OBJECT(BMUXlib_TS_ASP_P_Context)
   BMUXlib_TS_CreateSettings stCreateSettings;
   BMMA_Block_Handle hTSPacketBufferBlock;

   BMUXlib_TS_MuxSettings stMuxSettings;
   BMUXlib_TS_StartSettings stStartSettings;
   BMUXlib_TS_FinishSettings stFinishSettings;

   struct
   {
      struct
      {
         unsigned uiWriteOffset; /* Next entry to be written by host */
         unsigned uiShadowReadOffset; /* Next entry to be queued to FW */
         unsigned uiReadOffset; /* Oldest entry still queued in FW */

         BMUXlib_TS_ASP_P_SystemData_Entry astEntry[BMUXLIB_TS_MAX_SYSTEM_DATA_COUNT];
      } stInputQueue; /* Stores the buffers queued from application via AddSystemDataBuffers() */

      size_t uiCompletedCount;
      bool bNextExpectedESCRValid;
      uint64_t uiNextExpectedESCR;
   } stSystemData;

   struct
   {
      BAVC_VideoBufferDescriptor astDescriptor[BMUXLIB_TS_MAX_VIDEO_FRAME_COUNT];

      BMUXlib_ASP_TS_Userdata_Host_Interface_t *pstQueue;
      BMMA_Block_Handle hBlock;
      BMMA_DeviceOffset uiOffset;
      unsigned uiShadowReadOffset; /* Next entry to be freed by FW */
   } stUserData;

   struct
   {
      bool bVideo[BMUXLIB_TS_MAX_VIDEO_PIDS];
      bool bAudio[BMUXLIB_TS_MAX_AUDIO_PIDS];
   } stInputReady;
   bool bAllInputsReady;

   BMUXlib_State eState;

   BMUXlib_TS_ASP_P_MemoryBlock stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eMax];
   BMUXlib_ASP_TS_Channel_Context_t stAspChannelContext;

   BMUXlib_TS_ASP_Interface stASPInterface;

#if BMUXLIB_TS_ASP_P_HAS_ASP
   BASP_Mux_TS_ChannelStartMessage stAspStartMessage;
#endif
} BMUXlib_TS_ASP_P_Context;

#define BMUXLIB_TS_ASP_P_GET_MUX_STATE(_hMux) (_hMux->eState)
#define BMUXLIB_TS_ASP_P_SET_MUX_STATE(_hMux, _state) (_hMux->eState = _state)

#endif /* MAGNUM_SYSLIB_MUXLIB_SRC_STREAM_TS_BMUXLIB_TS_ASP_PRIV_H_ */
