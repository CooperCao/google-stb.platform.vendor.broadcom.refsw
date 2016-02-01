/***************************************************************************
 *     Copyright (c) 2004-2005, Broadcom Corporation
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
 *   See Module Overview below.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BXVD_REG_H__
#define BXVD_REG_H__

#include "bxvd.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t BXVD_Reg_Read32_isr(
    BXVD_Handle hXvd, /* XVD handle */
    uint32_t offset   /* Register offset to read */
    );

void BXVD_Reg_Write32_isr(
    BXVD_Handle hXvd, /* XVD handle */
    uint32_t offset,  /* Memory offset to write */
    uint32_t data     /* Data */
    );

uint32_t BXVD_Reg_Read32(
    BXVD_Handle hXvd, /* XVD handle */
    uint32_t offset   /* Register offset to read */
    );

void BXVD_Reg_Write32(
    BXVD_Handle hXvd, /* XVD handle */
    uint32_t offset,  /* Memory offset to write */
    uint32_t data     /* Data */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXVD_REG_H__ */
