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
 *****************************************************************************/

#ifndef BMUXLIB_ASP_SOURCE_AV_CONTEXT_H_
#define BMUXLIB_ASP_SOURCE_AV_CONTEXT_H_

#include "bmuxlib_asp_ts_consts.h"
#include "bmuxlib_asp_source_itb.h"

typedef enum BMUXlib_ASP_TS_Source_AV_State_e
{
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_GET_FIRST_ITB,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_GET_NEXT_ITB,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_GENERATE,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_DMA_START,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_DMA_FINISH,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_SEND,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_GENERATE,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_DMA_START,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_DMA_FINISH,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_SEND,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_PAYLOAD,
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_EOS,

   /* Add more enums ABOVE this line */
   BMUXLIB_ASP_TS_SOURCE_AV_STATE_MAX
} BMUXlib_ASP_TS_Source_AV_State_e;

typedef enum BMUXlib_TS_ASP_Source_AV_ITB_State_e
{
   BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_READ,
   BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_DMA_START,
   BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_DMA_FINISH,
   BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_VALID,

   /* Add more enums ABOVE this line */
   BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_MAX
} BMUXlib_TS_ASP_Source_AV_ITB_State_e;

#define BMUXLIB_ASP_TS_MAX_SAVED_ITB 2

typedef enum BMUXlib_TS_ASP_Source_AV_ITBEntry_Type
{
   BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eBase = 0x01,
   BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eTimestamp = 0x02,
   BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eBitrate = 0x04,
   BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eESCR = 0x08,

   BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eAll = 0x0F
} BMUXlib_TS_ASP_Source_AV_ITBEntry_Type;

typedef struct BMUXlib_TS_ASP_Source_AV_Frame_t
{
   uint64_t uiCDBOffset;

   uint32_t uiESCR;
   uint32_t uiTicksPerBit;
   int16_t iSHR;

   uint64_t uiPTS;
   uint64_t uiDTS;
   uint32_t uiOriginalPTS;

   bool_t bRAI;

   uint8_t uiITBEntriesParsed;
} BMUXlib_TS_ASP_Source_AV_Frame_t;

typedef struct BMUXlib_TS_ASP_Source_AV_ITBEntry_t
{
   BMUXlib_TS_ASP_Source_AV_ITB_State_e eITBState;
   BMUXlib_TS_ASP_Source_AV_Frame_t stFrameInfo;
#if 0
   uint32_t auiData[BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eMax][BMUXLIB_ASP_ITB_ENTRY_SIZE_IN_WORDS];
   uint8_t uiCurrentIndex;
   uint32_t *astEntry[BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eMax];
#endif
   uint8_t uiITBSize;

   void *pDMAToken;
} BMUXlib_TS_ASP_Source_AV_ITBEntry_t;

typedef struct BMUXlib_ASP_TS_Source_AV_Context_t
{
   const BMUXlib_ASP_TS_Source_AV_Interface_t *pstSourceAVInterface;
   const BMUXlib_ASP_Register_Interface_t *pstRegisterInterface;
   const BMUXlib_ASP_DMA_Interface_t *pstDMAInterface;
   BMUXlib_ASP_TS_Source_AV_State_e eState;

   BMUXlib_ASP_Source_ContextValues_t stData; /* Actual snapshot of CDB register *VALUES* */
   BMUXlib_ASP_Source_ContextValues_t stShadowData; /* Shadow of CDB register *VALUES* */

   BMUXlib_ASP_Source_ContextValues_t stIndex; /* Actual snapshot of ITB register *VALUES* */
   BMUXlib_ASP_Source_ContextValues_t stShadowIndex; /* Shadow of ITB register *VALUES* */

   bool_t bRegistersInitialized;

   struct
   {
      BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstCurrentEntry;
      BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstNextEntry;
      BMUXlib_TS_ASP_Source_AV_ITBEntry_t astEntry[BMUXLIB_ASP_TS_MAX_SAVED_ITB];
   } stITB;

   struct
   {
      size_t uiFrameSize;
      uint8_t auiTSPacket[BMUXLIB_ASP_TS_PACKET_SIZE];
      uint64_t uiBufferOffset;
      uint32_t auiITBEntry[BMUXLIB_ASP_ITB_ENTRY_SIZE_IN_WORDS];
   } stTemp;

   void *pDMAToken;
} BMUXlib_ASP_TS_Source_AV_Context_t;

#endif /* BMUXLIB_ASP_SOURCE_AV_CONTEXT_H_ */
