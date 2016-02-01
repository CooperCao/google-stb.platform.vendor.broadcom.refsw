/***************************************************************************
 *	   Copyright (c) 2003-2005, Broadcom Corporation
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
 ***************************************************************************/

#ifndef BINT_DUMP_H
#define BINT_DUMP_H

#include "bint.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
Summary:
Dumps the stats for an interrupt handle.

Description:
Prints out the statistics for all callbacks and its bins that have been
registered under the specified interrupt handle.
*/
BERR_Code BINT_Stats_Dump
	( BINT_Handle          intHandle );

/*
Summary:
Dumps the stats for an interrupt handle in comma delimited data format.

Description:
Prints out the statistics for all callbacks and its bins that have been
registered under the specified interrupt handle, with commas seperating
data fields.
*/
BERR_Code BINT_Stats_DumpData
	( BINT_Handle          intHandle );

#ifdef __cplusplus
}
#endif
 
#endif
/* End of File */



