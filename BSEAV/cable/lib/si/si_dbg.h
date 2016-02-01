/***************************************************************************
 *     Copyright (c) 2002-2009, Broadcom Corporation
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

#ifndef SI_DBG_H
#define SI_DBG_H

/* need to have #include "si_os.h" before inclusion of this file. */

typedef enum
{
	E_SI_DBG_MSG,
	E_SI_WRN_MSG,
	E_SI_ERR_MSG,
}SI_DEBUGLEVELS;


#ifdef __cplusplus
extern "C" {
#endif

void SI_Dbg_Set_Debug_Level (unsigned long level);
unsigned long SI_Dbg_Get_Debug_Level (void);

#ifdef __cplusplus
}
#endif

#define SI_DBG_PRINT(x,y)	if (x >= SI_Dbg_Get_Debug_Level()) SI_print y

#endif
