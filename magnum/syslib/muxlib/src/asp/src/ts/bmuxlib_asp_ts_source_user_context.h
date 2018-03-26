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

#ifndef BMUXLIB_ASP_SOURCE_USER_CONTEXT_H_
#define BMUXLIB_ASP_SOURCE_USER_CONTEXT_H_

typedef enum BMUXlib_ASP_TS_Source_User_State_e
{
   BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_WAIT, /* Wait for a new user data entry to arrive in the queue */
   BMUXLIB_ASP_TS_SOURCE_USER_STATE_QUEUE_DMA_FINISH, /* Wait for local DCCM write pointers to get updated from DRAM */
   BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_DMA_START, /* Start the DMA the user data entry to DCCM */
   BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_DMA_FINISH, /* Wait for the DMA of the user data entry to DCCM */
   BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_PAYLOAD, /* Queue the user data entry's current buffer to the output */
   BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_BUFFER_DONE, /* Current Buffer of Payload is done */

   /* Add more enums ABOVE this line */
   BMUXLIB_ASP_TS_SOURCE_USER_STATE_PROCESS_MAX
} BMUXlib_ASP_TS_Source_User_State_e;

typedef struct BMUXlib_TS_ASP_Source_User_Context_t
{
   BMUXlib_ASP_TS_Source_User_State_e eState;
   uint16_t uiPIDChannelIndex;

   struct
   {
      uint64_t uiBaseOffset; /* Offset to start of Queue */
      uint32_t uiShadowRead;
      bool_t *pbReadOffsetUpdated;
      uint32_t *puiReadOffset;
      const uint32_t *puiWriteOffset;
   } stQueue;

   struct
   {
      BMUXlib_ASP_TS_Userdata_Entry_t stEntry;
      uint8_t uiCurrentBuffer;
      unsigned uiBytesQueued;
   } stCurrent;

   void *pDMAToken;
} BMUXlib_ASP_TS_Source_User_Context_t;

typedef struct BMUXlib_ASP_TS_Source_User_Common_Context_t
{
   void *pDMATokenWriteOffset; /* DMA Token for reading the write offsets from DRAM to DCCM */
   uint32_t uiWriteOffset[BMUXLIB_ASP_TS_MAX_USERDATA_SOURCE];

   bool_t bReadOffsetUpdated;
   void *pDMATokenReadOffset; /* DMA Token for writing the read offsets from DCCM to DRAM */
   uint32_t uiReadOffset[BMUXLIB_ASP_TS_MAX_USERDATA_SOURCE];
} BMUXlib_ASP_TS_Source_User_Common_Context_t;

#endif /* BMUXLIB_ASP_SOURCE_USER_CONTEXT_H_ */
