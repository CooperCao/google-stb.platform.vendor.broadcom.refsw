/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
#include "bchp.h"
#include "bint.h"
#include "breg_mem.h"
#include "bmem.h"
#include "btmr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
/***********************************************************************
 *                      Interrupt Handles
 ***********************************************************************/
BINT_Handle bcmGetIntHandle		(void);
BINT_Handle bcmGetUpgIntHandle	(void);
#ifdef DIAG_FE_BCM3250
BINT_Handle bcmGet3250IntHandle		(void);
#endif

/***********************************************************************
 *                      Register Handles
 ***********************************************************************/
BREG_Handle bcmGetRegHandle		(void);
#ifdef DIAG_FE_BCM3250
BREG_Handle bcmGet3250RegHandle		(void);
#endif

/***********************************************************************
 *                      Chip Handles
 ***********************************************************************/
BCHP_Handle bcmGetChipHandle	(void);
#ifdef DIAG_FE_BCM3250
BCHP_Handle bcmGet3250ChipHandle	(void);
#endif

/***********************************************************************
 *                      Mem Handle
 ***********************************************************************/
/*BMEM_Handle bcmGetMemHandle			(void);*/

/***********************************************************************
 *                      Flash Handle
 ***********************************************************************/
BREG_Handle bcmGetFlashRegHandle	(void);

/***********************************************************************
 *                      Open Handles
 ***********************************************************************/
BERR_Code bcmOpenHandles(void);

extern BINT_Handle diags_hInt;
extern BREG_Handle diags_hReg;
extern BCHP_Handle diags_hChip;
extern BREG_Handle diags_hFlash;
extern BTMR_Handle diags_hTimer;

#ifdef __cplusplus
}
#endif

