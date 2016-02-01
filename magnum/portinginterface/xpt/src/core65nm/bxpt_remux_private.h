/***************************************************************************
 *     Copyright (c) 2003-2005, Broadcom Corporation
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
 * Private function definitions for the remux modules. 
 *
 * Revision History:
 *
 *
 ***************************************************************************/

#ifndef BXPT_REMUX_PRIVATE_H__
#define BXPT_REMUX_PRIVATE_H__

#include "bxpt.h"
#include "bxpt_remux.h"

#ifdef __cplusplus
extern "C" {
#endif

void BXPT_Remux_P_WriteReg(
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	uint32_t Reg0Addr,
	uint32_t RegVal
	);

uint32_t BXPT_Remux_P_ReadReg(
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	uint32_t Reg0Addr
	);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_REMUX_PRIVATE_H__ */

/* end of file */



