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


#ifndef BRSP_PRIV_H__
#define BRSP_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif


#define	BRSP_MAGIC_ID						(0xBABEFACE)

typedef struct BRSP_P_MByte_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */
	BREG_Handle hReg;
	uint32_t coreOffset;
	uint32_t nbrShadowReg;
	uint32_t *pShadowRegs;
} BRSP_P_MByte_Handle;


#ifdef __cplusplus
}
#endif
 
#endif



