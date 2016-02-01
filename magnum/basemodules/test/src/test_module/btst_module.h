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

#ifndef BTST_MODULE_H__
#define BTST_MODULE_H__

#include "bchp.h"
#include "breg_mem.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct BTST_HandleImpl *BTST_Handle;

typedef struct {
    uint32_t dummy;
} BTST_Settings;

BERR_Code BTST_GetDefaultSettings(BTST_Settings *pDefSettings);
BERR_Code BTST_Open(BTST_Handle *phTst, BCHP_Handle hChip, BREG_Handle hRegister, const BTST_Settings *pDefSettings);
BERR_Code BTST_Reset(BTST_Handle hTst, const BTST_Settings *pDefSettings);
BERR_Code BTST_Close(BTST_Handle hTst);
BERR_Code BTST_DoTest(BTST_Handle hTst);


#ifdef __cplusplus
}
#endif

#endif  /* BTST_MODULE_H__ */
