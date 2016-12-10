/******************************************************************************
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
 *****************************************************************************/

#include "bstd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd.h"
#include "bxvd_reg.h"

BDBG_MODULE(BXVD_REG);

#define BXVD_REG_READ_MAX_RETRIES            10
#define BXVD_REG_WRITE_POST_FAILURE_DELAY    5 /* wait this many usecs for the write to complete */

uint32_t BXVD_Reg_Read32_isr(BXVD_Handle hXvd, uint32_t offset)
{
   uint32_t uiValue;

   uiValue = BREG_Read32_isr(hXvd->hReg, offset);

   return uiValue;
}

void BXVD_Reg_Write32_isr(BXVD_Handle hXvd, uint32_t offset, uint32_t data)
{

   BREG_Write32_isr(hXvd->hReg, offset, data);
}

uint32_t BXVD_Reg_Read32(BXVD_Handle hXvd, uint32_t offset)
{
   uint32_t uiValue;

   uiValue = BREG_Read32(hXvd->hReg, offset);

   return uiValue;
}

void BXVD_Reg_Write32(BXVD_Handle hXvd, uint32_t offset, uint32_t data)
{
   BREG_Write32(hXvd->hReg, offset, data);
}

#if BXVD_P_CORE_40BIT_ADDRESSABLE
uint64_t BXVD_Reg_Read64(BXVD_Handle hXvd, uint64_t offset)
{
   uint64_t uiValue;

   uiValue = BREG_Read64(hXvd->hReg, offset);

   return uiValue;
}

void BXVD_Reg_Write64(BXVD_Handle hXvd, uint64_t offset, uint64_t data)
{
   BREG_Write64(hXvd->hReg, offset, data);
}
#endif
