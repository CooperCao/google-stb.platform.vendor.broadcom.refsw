/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
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

/*= Module Overview *********************************************************
<verbatim>

Overview
This common utility module is an extension of 'BRSP' common utility.  This
common utility is specifically for multi-byte devices.  A multi-byte device
is defined as a device like BQDS and BQOB.  Since this common utility 
is an extension of 'BRSP', it provides all the functional of 'BRSP'.
See 'BRSP' documentation for a more detail description.


Design
None


Usage
The usages of BRSP_MByte involves the following:

	* Create Register Script (statically or dynamically)
	* Create BRSP_MByte Handle
	* Call parse function to parse Register Script


Sample Code
// The following is simple example that writes to several registers
// in the Bcm3250 Qam InBand Downstream core.
static const BRSP_ScriptTbl regScript[] =
{
	BRSP_ScriptType_eWrite8, SB_FRZ1,			0x000000FF,
	BRSP_ScriptType_eWrite8, SB_FRZ2,			0x000000FF,
	BRSP_ScriptType_eWrite8, SB_RST1,			0x0000007F,
	BRSP_ScriptType_eWrite8, SB_RST2,			0x000000FF,
	BRSP_ScriptType_eWrite8, SB_AGI,			0x0000000D,
	BRSP_ScriptType_eWrite8, SB_AGT,			0x0000001D,
	BRSP_ScriptType_eMbWrite32, MB_STPGA,		0x80000000,
	BRSP_ScriptType_eMbWrite32, MB_STAGI,		0x40000000,
	BRSP_ScriptType_eMbWrite32, MB_STAII,		0x3FFF0000,
	BRSP_ScriptType_eMbWrite32, MB_STAGT,		0xFC000000,
	BRSP_ScriptType_eMbWrite32, MB_STAIT,		0x80000000,
	BRSP_ScriptType_eMbWrite32, MB_STABW,		0x00000D0B,
	BRSP_ScriptType_eMbWrite32, MB_STATHR,		0x00000480,
	BRSP_ScriptType_eMbWrite32, MB_STAFTHR,		0x080D0000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static BCHP_Handle hChip3250;
static BREG_Handle hReg3250;
static BINT_Handle hInt7038;

main( void )
{
	BRSP_MByte_Handle hRsp;

	// Initialize hChip3250, hReg3250, and hInt7038 . . .

	BRSP_ParseRegScript( hReg7038, regScript, NULL );

	// Create BRSP_MByte_Handle.  0x200 = Qam InBand Downstream core offset
	// 0xC8 = number of Qam InBand Downstream Multi-byte registers
	BRSP_MByte_Open( &hRsp, hReg3250, 0x200, 0xC8 );
	BRSP_MByte_ParseRegScript( hRsp, regScript, NULL );
}

</verbatim>
***************************************************************************/


#ifndef BRSP_MBYTE_H__
#define BRSP_MBYTE_H__

#include "brsp.h"
#include "breg_mem.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
	The handles for BRSP_MByte module.

Description:
	An opaque handle for BRSP_MByte module.

See Also:
	BRSP_MByte_Open(), BRSP_MByte_Close()

****************************************************************************/
typedef struct BRSP_P_MByte_Handle		*BRSP_MByte_Handle;


/***************************************************************************
Summary:
	This function creates a BRSP_MByte handle.

Description:
	This function is responsible for creating BRSP_MByte handle.
	Using the created handle, one can do Multi-Byte read/write function to
	a device that is Multi-Byte compatible.

Returns:
	TODO:

See Also:
	BRSP_MByte_Close()

****************************************************************************/
BERR_Code BRSP_MByte_Open(
	BRSP_MByte_Handle *phRsp,				/* [out] MBtye Register Handle */
	BREG_Handle hReg,						/* [in] Register Handle to use */
	uint32_t coreOffset,					/* [in] Core Offset */
	uint32_t nbrWriteOnlyShadowReg			/* [in] Number of Write Only Register, 0 if none */
	);

/***************************************************************************
Summary:
	This function destroys a BRSP_MByte handle.

Description:
	This function is responsible for destorying BRSP_MByte handle.
	BRSP_MByte handle should be created using the create function.

Returns:
	TODO:

See Also:
	BRSP_MByte_Open()

****************************************************************************/
BERR_Code BRSP_MByte_Close(
	BRSP_MByte_Handle hRsp					/* [in] MBtye Register Handle */
	);

/***************************************************************************
Summary:
	This function writes a 32bit value to a BRSP_MByte handle.

Description:
	This function is responsible for writing a 32bit value to a 
	BRSP_MByte handle.  The BRSP_MByte handle must be created using
	the create function.

Returns:
	TODO:

See Also:
	BRSP_MByte_Open()

****************************************************************************/
void BRSP_MByte_Write32(
	BRSP_MByte_Handle hRsp,					/* [in] MBtye Register Handle */
	uint32_t opCode,						/* [in] MByte Operation Code to write to (see chip doc.) */
	uint32_t value							/* [in] Value to write to MByte Register */
	);

/***************************************************************************
Summary:
	This function reads a 32bit value from a BRSP_MByte handle.

Description:
	This function is responsible for reading a 32bit value from a 
	BRSP_MByte handle.  The BRSP_MByte handle must be created using
	the Open function.

Returns:
	TODO:

See Also:
	BRSP_MByte_Open()

****************************************************************************/
uint32_t BRSP_MByte_Read32(
	BRSP_MByte_Handle hRsp,					/* [in] MByte Register Handle */
	uint32_t opCode							/* [in] MByte Operation Code to read from (see chip doc.) */
	);

/***************************************************************************
Summary:
	This function reads a 32bit value from a BRSP_MByte handle of a
	write-only register.

Description:
	This function is responsible for reading a 32bit value from a 
	BRSP_MByte handle of a write-only register.
	The BRSP_MByte handle must be created using	the Open function.

Returns:
	TODO:

See Also:
	BRSP_MByte_Read32()

****************************************************************************/
uint32_t BRSP_MByte_Read32WriteOnly(
	BRSP_MByte_Handle hRsp,					/* [in] MByte Register Handle */
	uint32_t opCode							/* [in] MByte Operation Code to read from (see chip doc.) */
	);

/***************************************************************************
Summary:
	This function parses a BRSP script with MByte registers.

Description:
	This function is responsible for parsing a BRSP script with MByte
	registers.

Returns:
	TODO:

See Also:
	None

****************************************************************************/
BERR_Code BRSP_MByte_ParseRegScript(
	BRSP_MByte_Handle hRsp,					/* [in] Handle to Register Script */
	const BRSP_ScriptTbl *pScriptTbl,		/* [in] Pointer to Register Script Tbl */
	void *phDev								/* [in] Handle of calling device */
	);


#ifdef __cplusplus
}
#endif
 
#endif



