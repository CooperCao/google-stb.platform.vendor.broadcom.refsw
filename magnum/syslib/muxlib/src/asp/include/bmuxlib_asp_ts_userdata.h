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

#ifndef BMUXLIB_ASP_TS_USERDATA_H_
#define BMUXLIB_ASP_TS_USERDATA_H_

#include "bmuxlib_asp_capabilities.h"

typedef struct BMUXlib_ASP_TS_Userdata_Buffer_t
{
   uint32_t uiOffsetHi;
   uint32_t uiOffsetLo;
   uint32_t uiSize;
} BMUXlib_ASP_TS_Userdata_Buffer_t;

#define BMUXLIB_ASP_TS_USERDATA_MAX_BUFFERS_PER_ENTRY 4

#define BMUXLIB_ASP_TS_USERDATA_BUFFER_INFO_NUM_VALID_BUFFERS_MASK 0x00000003
#define BMUXLIB_ASP_TS_USERDATA_BUFFER_INFO_NUM_VALID_BUFFERS_SHIFT 0

#define BMUXLIB_ASP_TS_USERDATA_BUFFER_INFO_RESERVED_MASK 0xFFFFFFFC
#define BMUXLIB_ASP_TS_USERDATA_BUFFER_INFO_RESERVED_SHIFT 2

typedef struct BMUXlib_ASP_TS_Userdata_Entry_t
{
   BMUXlib_ASP_TS_Userdata_Buffer_t stBuffer[BMUXLIB_ASP_TS_USERDATA_MAX_BUFFERS_PER_ENTRY];
   uint32_t uiBufferInfo; /* [31:02] reserved
                           * [01:00] uiNumValidBuffers - 1
                           */
   uint32_t uiESCR;
   uint64_t uiPTS; /* PTS for audio, DTS for video, and DTS equivalent for system/user data */
} BMUXlib_ASP_TS_Userdata_Entry_t;

typedef struct BMUXlib_ASP_TS_Userdata_Queue_t
{
   BMUXlib_ASP_TS_Userdata_Entry_t stEntry[BMUXLIB_ASP_TS_MAX_USERDATA_ENTRIES_PER_QUEUE];
} BMUXlib_ASP_TS_Userdata_Queue_t;

typedef struct BMUXlib_ASP_TS_Userdata_Host_Interface_t
{
   BMUXlib_ASP_TS_Userdata_Queue_t stQueue[BMUXLIB_ASP_TS_MAX_USERDATA_SOURCE];
   uint32_t uiReadOffset[BMUXLIB_ASP_TS_MAX_USERDATA_SOURCE];
   uint32_t uiWriteOffset[BMUXLIB_ASP_TS_MAX_USERDATA_SOURCE];
} BMUXlib_ASP_TS_Userdata_Host_Interface_t;

typedef struct BMUXlib_ASP_TS_Userdata_Interface_t
{
   uint64_t uiOffset;
   uint8_t uiNumValidUserData;
} BMUXlib_ASP_TS_Userdata_Interface_t;

#endif /* BMUXLIB_ASP_TS_USERDATA_H_ */
