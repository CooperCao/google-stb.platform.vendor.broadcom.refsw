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
#ifndef BREG_MEM_OPT_H
#define BREG_MEM_OPT_H

#if BDBG_DEBUG_BUILD == 1
#error breg_mem_opt.h should only be included for release builds!!
#endif

#include "breg_mem_priv.h"


#define BREG_Write32    BREG_P_Write32
#define BREG_Write16    BREG_P_Write16
#define BREG_Write8     BREG_P_Write8

#define BREG_Read32     BREG_P_Read32
#define BREG_Read16     BREG_P_Read16
#define BREG_Read8      BREG_P_Read8

#endif
/* End of File */




