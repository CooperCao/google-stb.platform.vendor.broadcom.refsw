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
#include "bkni.h"
#include "breg_7411.h"

BDBG_MODULE(BREG_7411);



struct BREG_7411_Context {
	BREG_Handle base;
};

#define B_REG_ADDR_HI	0
#define B_REG_ADDR_LO	2
#define B_REG_DATA_HI	4
#define B_REG_DATA_LO	6

static void 
BREG_7411_SetAddr(struct BREG_7411_Context *cntx, uint32_t addr)
{
	uint16_t low, high;

	low = (uint16_t)(addr & 0xFFFF);
	high = (uint16_t) (addr >> 16);
	BREG_Write16(cntx->base, B_REG_ADDR_HI, high);
	BREG_Write16(cntx->base, B_REG_ADDR_LO, low);
	return;
}

static void
BREG_7411_Write32(void *context, uint32_t reg, uint32_t data)
{
	struct BREG_7411_Context *breg = context;
	uint16_t low, high;

	BREG_7411_SetAddr(breg, reg);
	low = (uint16_t)(data& 0xFFFF);
	high = (uint16_t) (data >> 16);
	BREG_Write16(breg->base, B_REG_DATA_HI, high);
	BREG_Write16(breg->base, B_REG_DATA_LO, low);
	return;
}

static uint32_t
BREG_7411_Read32(void *context, uint32_t reg)
{
	struct BREG_7411_Context *breg = context;
	uint16_t low, high;
	uint32_t data;

	BREG_7411_SetAddr(breg, reg);
	high = BREG_Read16(breg->base, B_REG_DATA_HI);
	low = BREG_Read16(breg->base, B_REG_DATA_LO);
	data = low | ((uint32_t)high << 16);
	return data;
}


static struct BREG_Virt_Impl BREG_7411 = {
	NULL,
	BREG_7411_Write32,
	NULL,
	NULL,
	BREG_7411_Read32,
	NULL,
	NULL
};


BERR_Code
BREG_7411_Open( BREG_Virt_Handle *pRegHandle, BREG_Handle base)
{
	struct BREG_7411_Context *cntx;
	cntx = BKNI_Malloc(sizeof(*cntx));
	if (!cntx) {
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}
	*pRegHandle = BKNI_Malloc(sizeof(**pRegHandle));
	if (!*pRegHandle) {
		BKNI_Free(cntx);
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}
	**pRegHandle = BREG_7411;
	(*pRegHandle)->context = cntx;
	cntx->base = base;

	return BERR_SUCCESS;
}


void
BREG_7411_Close(BREG_Virt_Handle regHandle)
{
	BKNI_Free(regHandle->context);
	BKNI_Free(regHandle);
	return;
}

