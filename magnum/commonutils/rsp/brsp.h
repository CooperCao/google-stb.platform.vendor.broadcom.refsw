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
This common utility module is responsible for providing service to
parse Register Scripts.  A Register Script is simple table that contains
register's address and the value to write to that register.  The script
supports write operations in 8/16/32 bits as well as Multi-Byte writes
(used in Qam Downstream/Out-of-Band).  Along with the write operation, the
script supports Delay and CallBack functions.  Delays are implemented
using a busy loops.  The CallBack functions are provided when a simple write
is not suffice to configure a register, for example, the register setting may
require a run-time conditional operation to determine the appropriate value
to configure it.

All BRSP CallBack function must contain five parameters, four script
provided parameters and one device handle.  CallBack function must
return BERR_Code.  If an BERR_SUCCESS is not returned, then 
script parser will terminate.


Design
None


Usage
The usage of BRSP involves the following:

	* Create Register Script (statically or dynamically)
	* Call parse function to parse Register Script


Sample Code
// The following is simple example that writes to several registers
// in the RFM core.
static const BRSP_ScriptTbl regScript[] = 
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF1A1, 0x12345678,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF1B1, 0xFF00FF00,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF1B2, 0xBABEFACE,
	BRSP_ScriptType_eEndOfScript, 0x00,			     0x00000000
};

static BCHP_Handle hChip7038;
static BREG_Handle hReg7038;
static BINT_Handle hInt7038;

main( void )
{
	// Initialize hChip7038, hReg7038, and hInt7038 . . .

	BRSP_ParseRegScript( hReg7038, regScript, NULL );
}


</verbatim>
***************************************************************************/


#ifndef BRSP_H__
#define BRSP_H__

#include "breg_mem.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
	The typedef for BRSP Script Table.

Description:
	This typedef represents the BRSP Script Table.

See Also:
	BRSP_ParseRegScript()

****************************************************************************/
typedef uint32_t						BRSP_ScriptTbl;

#define BRSP_RD_MOD_WR_32( reg, field, val )			BCHP_##reg, BCHP_MASK(reg, field), BCHP_FIELD_DATA(reg, field, val), 0x00000000, 0x00000000

/***************************************************************************
Summary:
	Enumeration for Script Type supported by BRSP.

Description:
	This enumeration defines the Script Type supported by BRSP.

See Also:
	BRSP_ParseRegScript()

****************************************************************************/
typedef enum
{
	BRSP_ScriptType_eEndOfScript,			/* End of Script */
	BRSP_ScriptType_eNestedScript,			/* Link to nested-script */
	BRSP_ScriptType_eDelay,					/* Busy Delay, in Milliseconds */
	BRSP_ScriptType_eFunc,					/* CallBack Function */
	BRSP_ScriptType_eWrite8,				/* 8 bit single word write */
	BRSP_ScriptType_eWrite16,				/* 16 bit single word write */
	BRSP_ScriptType_eWrite32,				/* 32 bit single word write */
	BRSP_ScriptType_eMbWrite32,				/* Multi-byte write, supported only for InBand and OutOfBand Qam cores */
	BRSP_ScriptType_eRdModWr32,				/* Read/Mod./Write, 32 bit single word, requires six fields */
	BRSP_ScriptType_eLast
} BRSP_ScriptType;

/***************************************************************************
Summary:
	This function parses a BRSP script.

Description:
	This function is responsible for parsing a BRSP script.

Returns:
	TODO:

See Also:
	BRSP_ScriptType

****************************************************************************/
BERR_Code BRSP_ParseRegScript(
	BREG_Handle hReg,					/* [in] Handle to Register */
	const BRSP_ScriptTbl *pScriptTbl,	/* [in] Pointer to Register Script Tbl */
	void *phDev							/* [in] Handle of calling device */
	);


#ifdef __cplusplus
}
#endif
 
#endif



