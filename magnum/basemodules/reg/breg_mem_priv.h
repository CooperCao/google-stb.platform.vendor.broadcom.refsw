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
#ifndef BREG_MEM_PRIV_H
#define BREG_MEM_PRIV_H

#define BREG_P_Write32(RegHandle, reg, data) (*((volatile uint32_t *)((uintptr_t)((RegHandle)->BaseAddr)+(reg)))=data)
#define BREG_P_Write16(RegHandle, reg, data) (*((volatile uint16_t *)((uintptr_t)((RegHandle)->BaseAddr)+(reg)))=data)
#define BREG_P_Write8(RegHandle, reg, data) (*((volatile uint8_t *)((uintptr_t)((RegHandle)->BaseAddr)+(reg)))=data)

#define BREG_P_Read32(RegHandle, reg) (*((volatile uint32_t *)((uintptr_t)((RegHandle)->BaseAddr)+(reg))))
#define BREG_P_Read16(RegHandle, reg) (*((volatile uint16_t *)((uintptr_t)((RegHandle)->BaseAddr)+(reg))))
#define BREG_P_Read8(RegHandle, reg) (*((volatile uint8_t *)((uintptr_t)((RegHandle)->BaseAddr)+(reg))))

#endif /* BREG_MEM_PRIV_H */
