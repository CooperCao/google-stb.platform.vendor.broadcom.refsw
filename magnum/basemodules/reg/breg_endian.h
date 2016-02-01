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
This module supplies standard macros that are used to correct endianess for
hardware registers that do not match the endian-ness of the CPU.  An example
of this would be a register that is always big endian regardless of whether
the CPU was little endian or big endian.

It is recommended that these macros be used on a variable after
the BREG_ReadXX() functions are called, NOT around the functions themselves.
This is to prevent executing the BREG_ReadXX() multiple times inside the
macro (which could also cause inconsistent results if the register happens
to change while executing the macro).

These macros depend on the BSTD_CPU_ENDIAN define to be set properly.

For a big endian CPU: BSTD_CPU_ENDIAN should equal BSTD_ENDIAN_BIG
For a little endian CPU: BSTD_CPU_ENDIAN should equal BSTD_ENDIAN_LITTLE
***************************************************************************/

#ifndef BREG_ENDIAN_H
#define BREG_ENDIAN_H

#ifdef __cplusplus
extern "C" {
#endif

/* 
This macro is used to swap bytes of a 32 bit value.
*/
#define BREG_SWAP32( a )  do{a=((a&0xFF)<<24|(a&0xFF00)<<8|(a&0xFF0000)>>8|(a&0xFF000000)>>24);}while(0)

/* 
This macro is used to swap bytes of a 16 bit value.
*/
#define BREG_SWAP16( a )  do{a=((a&0xFF)<<8|(a&0xFF00)>>8);}while(0)

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG

/* 
This macro is used to ensure that a 32-bit value is properly formatted for
a big endian register access.
*/
#define BREG_BE32( value ) 

/* 
This macro is used to ensure that a 16-bit value is properly formatted for
a big endian register access.
*/
#define BREG_BE16( value )

/* 
This macro is used to ensure that a 32-bit value is properly formatted for
a little endian register access.
*/
#define BREG_LE32( value ) BREG_SWAP32( value )

/* 
This macro is used to ensure that a 16-bit value is properly formatted for
a little endian register access.
*/
#define BREG_LE16( value ) BREG_SWAP16( value )

#else
#define BREG_BE32( value ) BREG_SWAP32( value )
#define BREG_BE16( value ) BREG_SWAP16( value )
#define BREG_LE32( value )
#define BREG_LE16( value )
#endif

#ifdef __cplusplus
}
#endif
 
#endif
/* End of File */



