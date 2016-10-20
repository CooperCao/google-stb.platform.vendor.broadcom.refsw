/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description:
 *
 ***************************************************************************/

#include "brdc_rul.h"
#include "brdc_private.h"

BDBG_MODULE(brdc_rul);

/***************************************************************************
 * This function builds the address RUL to write immediate value (device address) into address register;
 */
void BRDC_AddrRul_ImmToReg_isr
    ( uint32_t                       **pRulPtr,
      uint32_t                         ulAddrReg,
      BMMA_DeviceOffset                deviceAddress )
{
    uint32_t *ptr = *pRulPtr;
    *ptr++ = BRDC_P_OP_IMM_TO_ADDR();
    *ptr++ = BRDC_REGISTER(ulAddrReg);
    *BRDC_P_ADDR_PTR(ptr) = BRDC_P_ADDR_OFFSET(deviceAddress);
    *pRulPtr = (uint32_t*)(BRDC_P_ADDR_PTR(ptr) + 1);
}

/***************************************************************************
 * This function build the RUL to block write immediate values (device addresses) into
 * consecutive count of address registers;
 */
void BRDC_AddrRul_ImmsToRegs_isr
    ( uint32_t                       **pRulPtr,
      uint32_t                         ulStartAddrReg,
      uint32_t                         ulEndAddrReg,
      BMMA_DeviceOffset               *deviceAddress)
{
    size_t i;
    uint32_t count;
    uint32_t *ptr = *pRulPtr;

    BDBG_ASSERT(ulEndAddrReg >= ulStartAddrReg);
    count = (ulEndAddrReg - ulStartAddrReg)/BRDC_P_ADDR_REG_SIZE + 1;
    *ptr++ = BRDC_P_OP_IMMS_TO_ADDRS(count);
    *ptr++ = BRDC_REGISTER(ulStartAddrReg);
    for(i = 0; i < count; i++) {
        *BRDC_P_ADDR_PTR(ptr) = BRDC_P_ADDR_OFFSET(deviceAddress[i]);
        ptr = (uint32_t*)(BRDC_P_ADDR_PTR(ptr) + 1);
    }
    *pRulPtr = ptr;
}

/***************************************************************************
 * This function builds the address RUL to write immediate value into a local RDC variable;
 */
void BRDC_AddrRul_ImmToVar_isr
    ( uint32_t                       **pRulPtr,
      uint64_t                         imm,
      unsigned                         var )
{
    uint32_t *ptr = *pRulPtr;
    *ptr++ = BRDC_P_OP_IMM_TO_VAR(var);
    *BRDC_P_ADDR_PTR(ptr) = BRDC_P_ADDR_OFFSET(imm);
    *pRulPtr = (uint32_t*)(BRDC_P_ADDR_PTR(ptr) + 1);
}

/***************************************************************************
 * This function builds the address RUL to load register into local RDC variable;
 */
void BRDC_AddrRul_RegToVar_isr
    ( uint32_t                       **pRulPtr,
      uint32_t                         reg,
      unsigned                         var )
{
    *(*pRulPtr)++ = BRDC_P_OP_REG_TO_VAR(var);
    *(*pRulPtr)++ = BRDC_REGISTER(reg);
}

/***************************************************************************
 * This function builds the address RUL to load register from a local RDC variable;
 */
void BRDC_AddrRul_VarToReg_isr
    ( uint32_t                       **pRulPtr,
      uint32_t                         reg,
      unsigned                         var )
{
    *(*pRulPtr)++ = BRDC_P_OP_VAR_TO_REG(var);
    *(*pRulPtr)++ = BRDC_REGISTER(reg);
}

/***************************************************************************
 * This function builds the address RUL to 'and' two RDC local variables and write into the firstlocal variable;
 */
void BRDC_AddrRul_AndToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src0,
      unsigned                         src1,
      unsigned                         dst )
{
    *(*pRulPtr)++ = BRDC_P_OP_VAR_AND_VAR_TO_VAR(src0, src1, dst);
}

/***************************************************************************
 * This function builds the address RUL to 'or' two RDC local variables and write into the destination local variable;
 */
void BRDC_AddrRul_OrToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src0,
      unsigned                         src1,
      unsigned                         dst )
{
    *(*pRulPtr)++ = BRDC_P_OP_VAR_OR_VAR_TO_VAR(src0, src1, dst);
}

/***************************************************************************
 * This function builds the address RUL to negate the value of a local RDC variable and write to destination local variable;
 */
void BRDC_AddrRul_NotToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src,
      unsigned                         dst )
{
    *(*pRulPtr)++ = BRDC_P_OP_NOT_VAR_TO_VAR(src, dst);
}

/***************************************************************************
 * This function builds the address RUL to sum two RDC local variables and write into the destination local variable;
 */
void BRDC_AddrRul_SumToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src0,
      unsigned                         src1,
      unsigned                         dst )
{
    *(*pRulPtr)++ = BRDC_P_OP_VAR_SUM_VAR_TO_VAR(src0, src1, dst);
}

/***************************************************************************
 * This function build the RUL to sum a RDC local variable with an immediate number
 * and write into the destination local variable;
 */
void BRDC_AddrRul_SumImmToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src,
      uint64_t                         imm,
      unsigned                         dst )
{
    uint32_t *ptr = *pRulPtr;
    *ptr++ = BRDC_P_OP_VAR_SUM_IMM_TO_VAR(src, dst);
    *BRDC_P_ADDR_PTR(ptr) = BRDC_P_ADDR_OFFSET(imm);
    *pRulPtr = (uint32_t*)(BRDC_P_ADDR_PTR(ptr) + 1);
}

/***************************************************************************
 * This function build the RUL to sum a RDC local variable with an immediate number
 * and write into the destination local variable;
 */
void BRDC_AddrRul_XorImmToVar_isr
    ( uint32_t                       **pRulPtr,
      unsigned                         src,
      uint64_t                         imm,
      unsigned                         dst )
{
    uint32_t *ptr = *pRulPtr;
    *ptr++ = BRDC_P_OP_XOR_SUM_IMM_TO_VAR(src, dst);
    *BRDC_P_ADDR_PTR(ptr) = BRDC_P_ADDR_OFFSET(imm);
    *pRulPtr = (uint32_t*)(BRDC_P_ADDR_PTR(ptr) + 1);
}

/* end of file */
