/***************************************************************************
 *     Copyright (c) 2003-2006, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bchp_qam.h"
#include "bint_3250.h"

BDBG_MODULE(interruptinterface_3250);

#define BINT_P_STATUS	GL_BCM3250_IRQ_PENDING
#define BINT_P_MASK		GL_BCM3250_IRQ_MASK

static void BINT_P_3250_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_3250_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static uint32_t BINT_P_3250_ReadMask( BREG_Handle regHandle, uint32_t baseAddr );
static uint32_t BINT_P_3250_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr );

static const BINT_P_IntMap bint_3250A0[] =
{
	/* Since all Bcm3250 interrupt are mapped to one L1 interrupt, therefore
	   table only contains one entry. The function that uses this table
	   is hardcoded to support only one entry, the first entry. */
	{ 33, BCM3250_IRQ_ID, 0, "3250L1" }, 
	{ -1, 0, 0, NULL }
};

static const BINT_Settings bcm3250IntSettings =
{
	NULL,				/* Bcm3250 doesn't support S/W produced interrupts */
	NULL,				/* Bcm3250 doesn't support S/W produced interrupts */
	BINT_P_3250_SetMask,
	BINT_P_3250_ClearMask,
	BINT_P_3250_ReadMask,
	BINT_P_3250_ReadStatus,
	bint_3250A0,
	"3250"
};

/***************************************************************************
*	Not supported on Bcm3250
* static void BINT_P_3250SetInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
* {
* 	BINT_SET_INT(regHandle, baseAddr, shift);
* 	return;
* }
* 
* static void BINT_P_3250ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
* {
* 	BINT_CLEAR_INT(regHandle, baseAddr, shift);
* 	return;
* }
***************************************************************************/

static void BINT_P_3250_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
	uint16_t usVal;

	BSTD_UNUSED(baseAddr);

	usVal = BREG_Read16( regHandle, BINT_P_MASK );
	usVal &= ~(1 << (shift));
	BREG_Write16( regHandle, BINT_P_MASK, usVal);
}

static void BINT_P_3250_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
	uint16_t usVal;

	BSTD_UNUSED(baseAddr);

	usVal = BREG_Read16( regHandle, BINT_P_MASK );
	usVal |= (1 << (shift));
	BREG_Write16( regHandle, BINT_P_MASK, usVal);
}

const BINT_Settings *BINT_3250_GetSettings( void )
{
	return &bcm3250IntSettings;
}

static uint32_t BINT_P_3250_ReadMask( BREG_Handle regHandle, uint32_t baseAddr )
{
	BSTD_UNUSED(baseAddr);

	return ~((uint32_t)BREG_Read16( regHandle, BINT_P_MASK )) & 0x0000FFFF;
}

static uint32_t BINT_P_3250_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr )
{
	BSTD_UNUSED(baseAddr);

	return ((uint32_t)BREG_Read16( regHandle, BINT_P_STATUS )) & 0x0000FFFF;
}

