/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ***************************************************************************/
#ifndef BRDC_RUL_H__
#define BRDC_RUL_H__

#include "brdc.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    This file provides 64/32-bit compatible utility functions to build RUL.

 */

/***************************************************************************
 * This function build the RUL to write immediate value (device address) into address register;
 */
void BRDC_AddrRul_ImmToReg_isr
    ( uint32_t                       **pRulPtr,
      uint32_t                         ulAddrReg,
      BMMA_DeviceOffset                deviceAddress );

/***************************************************************************
 * This function build the RUL to write immediate value (device address) into local RDC variable;
 */
void BRDC_AddrRul_ImmToVar_isr
    ( uint32_t                       **pRulPtr,
      uint64_t                         imm,
      unsigned                         var );

/***************************************************************************
 * This function build the RUL to load adress register into local RDC variable;
 */
void BRDC_AddrRul_RegToVar_isr
    ( uint32_t                       **pRulPtr,
      uint32_t                         reg,
      unsigned                         var );

/***************************************************************************
 * This function build the RUL to load address register from a local RDC variable;
 */
void BRDC_AddrRul_VarToReg_isr
    ( uint32_t                       **pRulPtr,
      uint32_t                         reg,
      unsigned                         var );

/***************************************************************************
 * This function build the RUL to and two RDC local variables and write into the firstlocal variable;
 */
void BRDC_AddrRul_AndToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src0,
      unsigned                         src1,
      unsigned                         dst );

/***************************************************************************
 * This function build the RUL to and two RDC local variables and write into the destination local variable;
 */
void BRDC_AddrRul_OrToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src0,
      unsigned                         src1,
      unsigned                         dst );

/***************************************************************************
 * This function build the RUL to negate the value of a local RDC variable and write to destination local variable;
 */
void BRDC_AddrRul_NotToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src,
      unsigned                         dst );

/***************************************************************************
 * This function build the RUL to and two RDC local variables and write into the destination local variable;
 */
void BRDC_AddrRul_SumToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src0,
      unsigned                         src1,
      unsigned                         dst );

/***************************************************************************
 * This function build the RUL to sum a RDC local variable with an immediate number
 * and write into the destination local variable;
 */
void BRDC_AddrRul_SumImmToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src,
      uint64_t                         imm,
      unsigned                         dst );

/***************************************************************************
 *  This function build the RUL to xor a RDC local variable with an immediate number
 * and write into the destination local variable;
 */
void BRDC_AddrRul_XorImmToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src,
      uint64_t                         imm,
      unsigned                         dst );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BRDC_RUL_H__ */

/* end of file */
