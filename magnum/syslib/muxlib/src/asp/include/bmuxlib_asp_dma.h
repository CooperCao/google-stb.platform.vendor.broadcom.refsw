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

#ifndef BMUXIB_ASP_DMA_H_
#define BMUXIB_ASP_DMA_H_

#include "bmuxlib_asp_types.h"

/* Module Overview:
 *
 * The DMA Interface is an abstraction to allow the mux manager to
 * copy data to/from DCCM from/to DRAM. When a DMA transfer is successfully
 * initiated (via BMUXlib_ASP_Dram2Dccm or BMUXlib_ASP_Dccm2Dram), a token
 * is returned.
 *
 * The token is useful to manage resources on systems where there are a
 * limited number of DMA channels and/or the DMA transfer is asynchronous.
 *
 * If a non-NULL token is returned, completion of the DMA can be checked by
 * passing the token to BMUXlib_ASP_Idle. The DMA is complete when
 * BMUXlib_ASP_Idle returns TRUE
 *
 * If all DMA channels are in-use the token will be NULL. When this occurs,
 * the caller is required to re-try the DMA transaction.
 */

/* BMUXlib_ASP_Dram2Dccm - Initiates a transfer of data from DRAM to DCCM
 *  pvContext: the DMA context
 *  pBuffer: the destination address in DCCM
 *  uiOffsetLo/Hi: the source address
 *  uiSize: the length of the transfer
 *  *pDMAToken: set to non-NULL if the DMA transfer was started. Must be passed to BMUXlib_ASP_Idle to see if the DMA finished.
 */
typedef void
(*BMUXlib_ASP_Dram2Dccm)(
   void *pvContext,
   void* pBuffer,
   uint64_t uiOffset,
   uint32_t uiSize,
   void **pDMAToken
   );

/* BMUXlib_ASP_Dccm2Dram - Initiates a transfer of data from DCCM to DRAM
 *  pvContext: the DMA context
 *  uiOffsetLo/Hi: the destination address
 *  pBuffer: the source address in DCCM
 *  uiSize: the length of the transfer
 *  *pDMAToken: set to non-NULL if the DMA transfer was started. Must be passed to BMUXlib_ASP_Idle to see if the DMA finished.
 */
typedef void
(*BMUXlib_ASP_Dccm2Dram)(
   void *pvContext,
   uint64_t uiOffset,
   const void* pBuffer,
   uint32_t uiSize,
   void **pDMAToken
   );

/* BMUXlib_ASP_Idle - Checks to see if a DMA transfer has completed
 *  pvContext: the DMA context
 *  *pDMAToken: Token returned by BMUXlib_ASP_Dram2Dccm or BMUXlib_ASP_Dccm2Dram
 */
typedef bool_t
(*BMUXlib_ASP_Idle)(
   void *pvContext,
   void **pDMAToken
   );

typedef struct BMUXlib_ASP_DMA_Interface_t
{
   void *pvContext;
   BMUXlib_ASP_Dram2Dccm fDram2Dccm;
   BMUXlib_ASP_Dccm2Dram fDccm2Dram;
   BMUXlib_ASP_Idle fIdle;
} BMUXlib_ASP_DMA_Interface_t;

#endif /* BMUXIB_ASP_DMA_H_ */
