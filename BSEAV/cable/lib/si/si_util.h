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

#ifndef SI_UTIL_H
#define SI_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned long SI_Construct_Data( unsigned char * rawdat, unsigned long idx, unsigned long num, unsigned long shift, unsigned long mask);
SI_RET_CODE SI_CRC32_Check ( unsigned char * data, unsigned short length);
void SI_Init_Section_Mask(unsigned long *mask, unsigned char last_section_number);
unsigned long SI_Chk_Section_mask(unsigned long *mask, unsigned char section_number);
void SI_Set_Section_mask(unsigned long *mask, unsigned char section_number);

#ifdef __cplusplus
}
#endif


#endif
