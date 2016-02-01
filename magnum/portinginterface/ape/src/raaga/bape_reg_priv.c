/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: APE Register Routines
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bape_priv.h"

BDBG_OBJECT_ID(BAPE_Reg_P_FieldList);

void BAPE_Reg_P_AddMaskValueToFieldList_isrsafe(BAPE_Reg_P_FieldList *pFieldList, uint32_t mask, uint32_t value)
{
    BDBG_OBJECT_ASSERT(pFieldList, BAPE_Reg_P_FieldList);
    pFieldList->mask |= mask;
    pFieldList->value &= ~mask;
    pFieldList->value |= value;    
}

void BAPE_Reg_P_ApplyFieldList(BAPE_Reg_P_FieldList *pFieldList, uint32_t address)
{
    BDBG_OBJECT_ASSERT(pFieldList, BAPE_Reg_P_FieldList); 
    BAPE_Reg_P_Update(pFieldList->hApe, address, pFieldList->mask, pFieldList->value); 
    BDBG_OBJECT_DESTROY(pFieldList, BAPE_Reg_P_FieldList);
}

void BAPE_Reg_P_ApplyFieldListAtomic(BAPE_Reg_P_FieldList *pFieldList, uint32_t address)
{
    BDBG_OBJECT_ASSERT(pFieldList, BAPE_Reg_P_FieldList); 
    BAPE_Reg_P_UpdateAtomic(pFieldList->hApe, address, pFieldList->mask, pFieldList->value); 
    BDBG_OBJECT_DESTROY(pFieldList, BAPE_Reg_P_FieldList);
}

void BAPE_Reg_P_ApplyFieldList_isr(BAPE_Reg_P_FieldList *pFieldList, uint32_t address)
{
    BDBG_OBJECT_ASSERT(pFieldList, BAPE_Reg_P_FieldList); 
    BAPE_Reg_P_Update_isr(pFieldList->hApe, address, pFieldList->mask, pFieldList->value); 
    BDBG_OBJECT_DESTROY(pFieldList, BAPE_Reg_P_FieldList);
}

void BAPE_Reg_P_ApplyFieldListAtomic_isr(BAPE_Reg_P_FieldList *pFieldList, uint32_t address)
{
    BDBG_OBJECT_ASSERT(pFieldList, BAPE_Reg_P_FieldList); 
    BAPE_Reg_P_UpdateAtomic_isr(pFieldList->hApe, address, pFieldList->mask, pFieldList->value); 
    BDBG_OBJECT_DESTROY(pFieldList, BAPE_Reg_P_FieldList);
}

