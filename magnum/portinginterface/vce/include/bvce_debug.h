/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BVCE_DEBUG_H_
#define BVCE_DEBUG_H_

#include "bavc.h"
#include "bavc_vce.h"

/* BVCE_Debug_PrintLogMessageEntry - prints the debug log entry to the console.
 *
 * The following messages can be enabled:
 *  BVCE_DBG_CFG - Encoder Configuration Changes
 *  BVCE_DBG_STS - Encoder Status
 *  BVCE_DBG_BUF - Buffer Descriptors sent by VCE PI
 *  BVCE_DBG_MTA - Metadata Descriptors sent by VCE PI
 *  BVCE_DBG_ITB - Encoder Raw ITB data
 *  BVCE_DBG_CMD - Encoder Commands
 *  BVCE_DBG_RSP - Encoder Responses
 *  BVCE_DBG_CDO - CDB Offsets snapshot
 *  BVCE_DBG_ITO - ITB Offsets snapshot
 */
void
BVCE_Debug_PrintLogMessageEntry(
      const void *pstFifoEntry /* Should point to an element of size BVCE_Debug_FifoInfo.uiElementSize */
      );

/* BVCE_Debug_FormatLogHeader - formats the entry type (uiIndex) for writing to file.
 * This should be called continuously incrementing uiIndex, starting with uiIndex=0 until
 * the function returns false
 */
bool
BVCE_Debug_FormatLogHeader(
   unsigned uiIndex,
   char *szMessage,
   unsigned uiSize
   );

/* BVCE_Debug_FormatLogMessage - formats the debug log entry for writing to file.
 */
void
BVCE_Debug_FormatLogMessage(
   const void *pstFifoEntry, /* Should point to an element of size BVCE_Debug_FifoInfo.uiElementSize */
   char *szMessage,
   unsigned uiSize
   );

/* BVCE_Debug_GetEntrySize - returns size of a log entry */
unsigned
BVCE_Debug_GetEntrySize( void );

/* BVCE_Debug_PrintLogVideoDescriptor - pretty-prints a video descriptor */
void
BVCE_Debug_PrintLogVideoDescriptor(
      const BAVC_VideoBufferDescriptor *pstDescriptor,
      char szDebug[],
      signed *piBytesLeft
      );

#endif /* BVCE_DEBUG_H_ */
