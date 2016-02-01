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
#ifndef BREG_7411_H
#define  BREG_7411_H

#include "berr.h"
#include "breg_virt.h"
#include "breg_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

BERR_Code BREG_7411_Open( BREG_Virt_Handle *pRegHandle, BREG_Handle base);
void BREG_7411_Close(BREG_Virt_Handle regHandle);

#ifdef __cplusplus
}
#endif

#endif /* BREG_7411_H */

