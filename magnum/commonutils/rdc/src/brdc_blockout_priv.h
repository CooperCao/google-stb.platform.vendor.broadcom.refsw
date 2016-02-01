/***************************************************************************
 *	   Copyright (c) 2003-2008, Broadcom Corporation
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
#ifndef BRDC__BLOCKOUT_PRIV_H__
#define BRDC__BLOCKOUT_PRIV_H__

#include "blst_list.h"			 /* Link list support */
#include "brdc.h"
#include "brdc_dbg.h"

/* For RDC block out RUL */
#define BRDC_P_RDC_BLOCKOUT_RUL_MAX_ENTRY	 0x4000

#ifdef __cplusplus
extern "C" {
#endif

BERR_Code BRDC_P_RdcBlockOutInit
	( BRDC_Handle			hRdc );

BERR_Code BRDC_P_RdcBlockOutDestroy
	( BRDC_Handle			hRdc );

BERR_Code BRDC_P_ValidateBlockOutRegisters
	( const BRDC_BlockOut *pstBlockOut,
	  uint32_t			   ulNumRegBlocks );

bool BRDC_P_IsRdcBlockOutEnabled_isr
	( BRDC_Handle			hRdc );

BERR_Code BRDC_P_ParseAndReplaceRul_isr
	( BRDC_List_Handle hList);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BRDC__BLOCKOUT_PRIV_H__ */


/* end of file */
