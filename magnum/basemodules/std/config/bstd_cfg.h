/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#ifndef BSTD_CFG_H__
#define BSTD_CFG_H__

/***************************************************************************
The following is a list of platform-specific defines. These are used to
create code that can be used for multiple chipsets/revisions, multiple 
families/revisions, and different board types. Porting interfaces and
SysLib authors should use them in Init or Open functions, not speckled
throughout the code. 

The user has two options:
-  Copy this file into their build environment and adjust the values to
   meet their platform specifications.
-  Provide values for the defines through makefile settings. In that case,
   this file may be used directly to satisfy the need for the bstd_cfg.h
   include.
****************************************************************************/

/* CPU endianness (choose one only) */
#if 0
#define BSTD_CPU_ENDIAN BSTD_ENDIAN_BIG
#define BSTD_CPU_ENDIAN BSTD_ENDIAN_LITTLE
#endif

/* enabling the debug build */
#if 0
#define BDBG_DEBUG_BUILD 1
#endif

/* chip type (choose one only) */
#if 0
#define BCHP_CHIP 7038
#endif

#endif /* #ifndef BSTD_CFG_H__ */

/* end of file */
