/***************************************************************************
 *     Copyright (c) 2002, Broadcom Corporation
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

/*================== Module Overview =====================================
This optional module is used in conjuction with the main FPGA module.
It is used to map textual representation of the various enumerations
used by the FPGA module (normally used to output current configurations
or provide debug information).
=============== End of Module Overview ===================================*/

#ifndef BFPGA_NAME__
#define BFPGA_NAME__

#include "bfpga.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
Summary:
This function returns the name of a given ts input.
*/
const char * BFPGA_GetTsSelectName( 
	BFPGA_TsSelect tsSelect /* Function returns text name of this ts select value */
	);

/*
Summary:
This function returns the name of a given FPGA output.
*/
const char * BFPGA_GetOutputSelectName( 
	BFPGA_OutputSelect outSelect /* Function returns text name of this out select value */
	);

/*
Summary:
This function dumps the current FPGA configure using the BDBG_ERR() function.
*/
void BFPGA_DumpConfiguration( 
	BFPGA_Handle hFpga /* handle to fpga */
	);

#ifdef __cplusplus
}
#endif

#endif

