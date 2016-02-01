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

/*= Module Overview ********************************************************
This module supplies the function required to access any type of registers.
Before any registers can be accessed an appropriate register handle should
be created (normally done at system init time).  This register handle is
then given to any modules that require access to the corresponding set of registers.
***************************************************************************/

#ifndef BREG_VIRT_H
#define BREG_VIRT_H

#ifdef __cplusplus
extern "C" {
#endif


/*
Summary:
This is an opaque handle that is used for memory mapped register functions.
*/
typedef struct BREG_Virt_Impl *BREG_Virt_Handle;

/*
Summary:
This function writes 32 bits to a register.

Description:
Although this fuction will never return an error it will assert if the
RegHandle is invalid.
*/
void BREG_Virt_Write32(
			      BREG_Virt_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
				  uint32_t reg, /* Register offset to write */
				  uint32_t data /* Data value to write to register */
				  );

/*
Summary:
This function writes 16 bits to a register.

Description:
Although this fuction will never return an error it will assert if the
RegHandle is invalid 
*/
void BREG_Virt_Write16(
			      BREG_Virt_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
				  uint32_t reg, /* Register offset to write */
				  uint16_t data /* Data value to write to register */
				  );

/*
Summary:
This function writes 8 bits to a register.

Description:
Although this fuction will never return an error it will assert if the
RegHandle is invalid.
*/
void BREG_Virt_Write8(
			      BREG_Virt_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
				  uint32_t reg, /* Register offset to write */
				 uint8_t data /* Data value to write to register */
				 );

/*
Summary:
This function reads 32 bits from a register.

Description:
Although this fuction cannot return an error it will assert if the
RegHandle is invalid.
*/
uint32_t BREG_Virt_Read32(
			     BREG_Virt_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
				 uint32_t reg /* Register offset to read */
				 );

/*
Summary:
This function reads 16 bits from a register.

Description:
Although this fuction cannot return an error it will assert if the
RegHandle is invalid.
*/
uint16_t BREG_Virt_Read16(
			     BREG_Virt_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
				 uint16_t reg /* Register offset to read */
				 );

/*
Summary:
This function reads 8 bits from a register.

Description:
Although this fuction cannot return an error it will assert if the
RegHandle is invalid.
*/
uint8_t BREG_Virt_Read8(
			     BREG_Virt_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
				 uint8_t reg /* Register offset to read */
				 );


#ifdef __cplusplus
}
#endif
 
#endif
/* End of File */
