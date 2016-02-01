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
#include "bchp_common.h"
#include "sys_handles.h"
#include "nexus_platform.h"
#include "priv/nexus_core.h"

#define HNDL_CHK_RETCODE( rc, func )        \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        return rc;                          \
    }                                       \
} while(0)

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

BCHP_Handle hChip = NULL;
BINT_Handle diags_hInt = NULL;
BREG_Handle diags_hReg = NULL;
BCHP_Handle diags_hChip = NULL;
/*BMEM_Handle diags_hMem = NULL;*/
BREG_Handle diags_hFlash = NULL;

/***********************************************************************
 *                       External Variables
 ***********************************************************************/

/***********************************************************************
 *                       External Functions
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

extern void bcmOpenFlashRegHandle (BREG_Handle *phFlash);

#if __cplusplus
}
#endif

/***********************************************************************
 *                      Interrupt Handles
 ***********************************************************************/
BINT_Handle bcmGetIntHandle     (void) { return diags_hInt;     }

/***********************************************************************
 *                      Register Handles
 ***********************************************************************/
BREG_Handle bcmGetRegHandle     (void) { return diags_hReg;     }

/***********************************************************************
 *                      Chip Handles
 ***********************************************************************/
BCHP_Handle bcmGetChipHandle    (void) { return diags_hChip;    }

/***********************************************************************
 *                      Flash Handle
 ***********************************************************************/
BREG_Handle bcmGetFlashRegHandle (void) { return diags_hFlash; }

/***********************************************************************
 *                      Mem Handle
 ***********************************************************************/
/*BMEM_Handle bcmGetMemHandle (void) { return diags_hMem; }*/

 /***********************************************************************
 *
 *  bcmOpenHandles()
 * 
 *  Open system handles (interrupt, register, chip handles)
 *
 ***********************************************************************/
BERR_Code bcmOpenHandles()
{
    diags_hReg = g_pCoreHandles->reg;
    diags_hChip = g_pCoreHandles->chp;
    diags_hInt = g_pCoreHandles->bint;
/*    diags_hMem = g_pCoreHandles->heap[0];*/
    return BERR_SUCCESS;
}
