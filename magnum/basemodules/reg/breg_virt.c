/***************************************************************************
 *	   Copyright (c) 2003, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "breg_virt.h"
#include "breg_virt_priv.h"


void 
BREG_Virt_Write32(BREG_Virt_Handle RegHandle, uint32_t reg, uint32_t data)
{
	BDBG_ASSERT(RegHandle->BREG_Write32);
	RegHandle->BREG_Write32(RegHandle->context, reg, data);
}

void 
BREG_Virt_Write16(BREG_Virt_Handle RegHandle, uint32_t reg, uint16_t data)
{
	BDBG_ASSERT(RegHandle->BREG_Write16);
	RegHandle->BREG_Write16(RegHandle->context, reg, data);
}

void 
BREG_Virt_Write8(BREG_Virt_Handle RegHandle, uint32_t reg, uint8_t data)
{
	BDBG_ASSERT(RegHandle->BREG_Write8);
	RegHandle->BREG_Write8(RegHandle->context, reg, data);
}

uint32_t
BREG_Virt_Read32(BREG_Virt_Handle RegHandle, uint32_t reg)
{
	BDBG_ASSERT(RegHandle->BREG_Read32);
	return RegHandle->BREG_Read32(RegHandle->context, reg);
}

uint16_t
BREG_Virt_Read16(BREG_Virt_Handle RegHandle, uint16_t reg)
{
	BDBG_ASSERT(RegHandle->BREG_Read16);
	return RegHandle->BREG_Read16(RegHandle->context, reg);
}

uint8_t
BREG_Virt_Read8(BREG_Virt_Handle RegHandle, uint8_t reg)
{
	BDBG_ASSERT(RegHandle->BREG_Read8);
	return RegHandle->BREG_Read8(RegHandle->context, reg);
}

