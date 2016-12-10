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
 * Module Description: APE Register Routines
 *
 ***************************************************************************/
#ifndef BAPE_REG_PRIV_H_
#define BAPE_REG_PRIV_H_

#include "breg_mem.h"

/* Compute Register Address */
#define BAPE_Reg_P_GetAddress(Register, Base, START, idx) (Register-Base + START(idx))

/* Compute Array Register Address */
#define BAPE_Reg_P_GetArrayAddress(ArrayName, ArrayIndex) (BDBG_ASSERT((ArrayIndex)<=BCHP_##ArrayName##_ARRAY_END),(BCHP_##ArrayName##_ARRAY_BASE)+((ArrayIndex)*((BCHP_##ArrayName##_ARRAY_ELEMENT_SIZE)/8)))

/* Read/Write registers */
#define BAPE_Reg_P_Read(hApe, regAddr) (BREG_Read32((hApe)->regHandle, (regAddr)))
#define BAPE_Reg_P_Write(hApe, regAddr, regVal) (BREG_Write32((hApe)->regHandle, (regAddr), (regVal)))

#define BAPE_Reg_P_Read_isr(hApe, regAddr) (BREG_Read32_isr((hApe)->regHandle, (regAddr)))
#define BAPE_Reg_P_Write_isr(hApe, regAddr, regVal) (BREG_Write32_isr((hApe)->regHandle, (regAddr), (regVal)))

#define BAPE_Reg_P_ReadField(hApe, regAddr, regName, fieldName)        BCHP_GET_FIELD_DATA(BAPE_Reg_P_Read    (hApe, regAddr),regName,fieldName)
#define BAPE_Reg_P_ReadField_isr(hApe, regAddr, regName, fieldName)    BCHP_GET_FIELD_DATA(BAPE_Reg_P_Read_isr(hApe, regAddr),regName,fieldName)

/* Update registers based on address, mask, value */
#define BAPE_Reg_P_Update(hApe, regAddr, regMask, regVal) (BREG_Write32((hApe)->regHandle, (regAddr), (BREG_Read32((hApe)->regHandle, (regAddr)) & ~(regMask)) | (regVal)))
#if !B_REFSW_MINIMAL
#define BAPE_Reg_P_UpdateAtomic(hApe, regAddr, regMask, regVal) (BREG_AtomicUpdate32((hApe)->regHandle, regAddr, regMask, regVal))
#endif

#define BAPE_Reg_P_Update_isr(hApe, regAddr, regMask, regVal) (BREG_Write32_isr((hApe)->regHandle, (regAddr), (BREG_Read32_isr((hApe)->regHandle, (regAddr)) & ~(regMask)) | (regVal)))
#if !B_REFSW_MINIMAL
#define BAPE_Reg_P_UpdateAtomic_isr(hApe, regAddr, regMask, regVal) (BREG_AtomicUpdate32_isr((hApe)->regHandle, regAddr, regMask, regVal))
#endif

/* Update single field of a register */
#define BAPE_Reg_P_UpdateField(hApe, regAddr, regName, fieldName, fieldData) (BAPE_Reg_P_Update(hApe, regAddr, BCHP_MASK(regName, fieldName), BCHP_FIELD_DATA(regName, fieldName, fieldData)))
#if !B_REFSW_MINIMAL
#define BAPE_Reg_P_UpdateFieldAtomic(hApe, regAddr, regName, fieldName, fieldData) (BAPE_Reg_P_UpdateAtomic(hApe, regAddr, BCHP_MASK(regName, fieldName), BCHP_FIELD_DATA(regName, fieldName, fieldData)))
#endif

#define BAPE_Reg_P_UpdateField_isr(hApe, regAddr, regName, fieldName, fieldData) (BAPE_Reg_P_Update_isr(hApe, regAddr, BCHP_MASK(regName, fieldName), BCHP_FIELD_DATA(regName, fieldName, fieldData)))
#if !B_REFSW_MINIMAL
#define BAPE_Reg_P_UpdateFieldAtomic_isr(hApe, regAddr, regName, fieldName, fieldData) (BAPE_Reg_P_UpdateAtomic_isr(hApe, regAddr, BCHP_MASK(regName, fieldName), BCHP_FIELD_DATA(regName, fieldName, fieldData)))
#endif

/* Update single enum field of a register */
#define BAPE_Reg_P_UpdateEnum(hApe, regAddr, regName, fieldName, fieldEnumVal) (BAPE_Reg_P_Update(hApe, regAddr, BCHP_MASK(regName, fieldName), BCHP_FIELD_ENUM(regName, fieldName, fieldEnumVal)))
#if !B_REFSW_MINIMAL
#define BAPE_Reg_P_UpdateEnumAtomic(hApe, regAddr, regName, fieldName, fieldEnumVal) (BAPE_Reg_P_UpdateAtomic(hApe, regAddr, BCHP_MASK(regName, fieldName), BCHP_FIELD_ENUM(regName, fieldName, fieldEnumVal)))
#endif

#define BAPE_Reg_P_UpdateEnum_isr(hApe, regAddr, regName, fieldName, fieldEnumVal) (BAPE_Reg_P_Update_isr(hApe, regAddr, BCHP_MASK(regName, fieldName), BCHP_FIELD_ENUM(regName, fieldName, fieldEnumVal)))
#if !B_REFSW_MINIMAL
#define BAPE_Reg_P_UpdateEnumAtomic_isr(hApe, regAddr, regName, fieldName, fieldEnumVal) (BAPE_Reg_P_UpdateAtomic_isr(hApe, regAddr, BCHP_MASK(regName, fieldName), BCHP_FIELD_ENUM(regName, fieldName, fieldEnumVal)))
#endif

/* Create some wrapper macros for the BCHP_ field macros.
   This is needed to allow for macro-expansion of arguments.
   Currently, BCHP_FIELD_ENUM, BCHP_MASK, and BCHP_SHIFT do NOT
   macro-expand their arguments.  */
#define BAPE_P_FIELD_ENUM(Register,Field,Name) \
        BCHP_FIELD_ENUM(Register,Field,Name)

#define BAPE_P_BCHP_MASK(Register,Field) \
        BCHP_MASK(Register,Field)

#define BAPE_P_BCHP_SHIFT(Register,Field) \
        BCHP_SHIFT(Register,Field)

#define BAPE_P_BCHP_ENUM_private(Register,Field,Name) \
        BCHP_##Register##_##Field##_##Name

#define BAPE_P_BCHP_ENUM(Register,Field,Name)  \
        BAPE_P_BCHP_ENUM_private(Register,Field,Name)      /* Using wrapper macro to provide macro-expansion of args */


/* Field List */
BDBG_OBJECT_ID_DECLARE(BAPE_Reg_P_FieldList);
typedef struct BAPE_Reg_P_FieldList
{
    BDBG_OBJECT(BAPE_Reg_P_FieldList)
    uint32_t mask, value;
    BAPE_Handle hApe;
}BAPE_Reg_P_FieldList;

/* Initialize Field List */
#define BAPE_Reg_P_InitFieldList(ApeHandle, FieldList) do { BDBG_OBJECT_SET(FieldList, BAPE_Reg_P_FieldList); (FieldList)->mask=0; (FieldList)->value=0; (FieldList)->hApe=(ApeHandle); } while(0)
#define BAPE_Reg_P_InitFieldList_isr BAPE_Reg_P_InitFieldList

/* Add Value to field list */
void BAPE_Reg_P_AddMaskValueToFieldList_isrsafe(BAPE_Reg_P_FieldList *pFieldList, uint32_t mask, uint32_t value);

#define BAPE_Reg_P_AddToFieldList(FieldList, RegName, FieldName, FieldData) BAPE_Reg_P_AddMaskValueToFieldList_isrsafe(FieldList, BCHP_MASK(RegName, FieldName), BCHP_FIELD_DATA(RegName, FieldName, FieldData))
#define BAPE_Reg_P_AddEnumToFieldList(FieldList, RegName, FieldName, FieldEnum) BAPE_Reg_P_AddMaskValueToFieldList_isrsafe(FieldList, BCHP_MASK(RegName, FieldName), BCHP_FIELD_ENUM(RegName, FieldName, FieldEnum))

#define BAPE_Reg_P_AddToFieldList_isr(FieldList, RegName, FieldName, FieldData) BAPE_Reg_P_AddMaskValueToFieldList_isrsafe(FieldList, BCHP_MASK(RegName, FieldName), BCHP_FIELD_DATA(RegName, FieldName, FieldData))
#define BAPE_Reg_P_AddEnumToFieldList_isr(FieldList, RegName, FieldName, FieldEnum) BAPE_Reg_P_AddMaskValueToFieldList_isrsafe(FieldList, BCHP_MASK(RegName, FieldName), BCHP_FIELD_ENUM(RegName, FieldName, FieldEnum))

/* Apply field list to a register */
void BAPE_Reg_P_ApplyFieldList(BAPE_Reg_P_FieldList *pFieldList, uint32_t address);
#if !B_REFSW_MINIMAL
void BAPE_Reg_P_ApplyFieldListAtomic(BAPE_Reg_P_FieldList *pFieldList, uint32_t address);
#endif

void BAPE_Reg_P_ApplyFieldList_isr(BAPE_Reg_P_FieldList *pFieldList, uint32_t address);
#if !B_REFSW_MINIMAL
void BAPE_Reg_P_ApplyFieldListAtomic_isr(BAPE_Reg_P_FieldList *pFieldList, uint32_t address);
#endif

#endif /* !defined BAPE_REG_PRIV_H_ */
