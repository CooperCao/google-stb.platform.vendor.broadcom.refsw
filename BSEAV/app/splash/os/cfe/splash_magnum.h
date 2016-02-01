/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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

#ifndef __SPLASH_MAGNUM_H__
#define __SPLASH_MAGNUM_H__

#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "lib_malloc.h"
#include "bchp_common.h"
#include "sbmips.h"

/* This header file is a extremely simplified version of the magnum stack */
/* Handles for example are either not required at all and are simplfied to nothing */
typedef void * BREG_Handle ;
typedef void * BMEM_Handle ;

#define REG_BASE ((void *)(PHYS_TO_K1(BCHP_PHYSICAL_OFFSET)))

#define BREG_Write32(RegHandle, reg, data) (*((volatile uint32_t *)((uintptr_t)REG_BASE+reg))=data)
#define BREG_Read32(RegHandle, reg) (*((volatile uint32_t *)((uintptr_t)REG_BASE+reg)))

/* Private pixel format types */
#define BPXL_P_ALPHA           0x01000000
#define BPXL_P_COLOR           0x02000000
#define BPXL_P_RGB             0x04000000
#define BPXL_P_YCbCr           0x08000000
#define BPXL_P_PALETTE         0x10000000
#define BPXL_P_SPECIAL         0x20000000
#define BPXL_P_WINDOW          0x40000000
#define BPXL_P_TYPE_MASK       0x7F000000

/* Private pixel component information */
#define BPXL_P_COMP_LOC_SHIFT  16

/* Private pixel component locations */
#define BPXL_P_LOC_A(p)        ((p) << (BPXL_P_COMP_LOC_SHIFT + 3 * 2))
#define BPXL_P_LOC_R(p)        ((p) << (BPXL_P_COMP_LOC_SHIFT + 2 * 2))
#define BPXL_P_LOC_G(p)        ((p) << (BPXL_P_COMP_LOC_SHIFT + 1 * 2))
#define BPXL_P_LOC_B(p)        ((p) << (BPXL_P_COMP_LOC_SHIFT + 0 * 2))
#define BPXL_P_LOC_Y(p)        ((p) << (BPXL_P_COMP_LOC_SHIFT + 2 * 2))
#define BPXL_P_LOC_Cb(p)       ((p) << (BPXL_P_COMP_LOC_SHIFT + 1 * 2))
#define BPXL_P_LOC_Cr(p)       ((p) << (BPXL_P_COMP_LOC_SHIFT + 0 * 2))
#define BPXL_P_LOC_ARGB        (BPXL_P_LOC_A(3)  | BPXL_P_LOC_R(2)  | BPXL_P_LOC_G(1)  | BPXL_P_LOC_B(0))
#define BPXL_P_LOC_AYCbCr      (BPXL_P_LOC_A(3)  | BPXL_P_LOC_Y(2)  | BPXL_P_LOC_Cb(1) | BPXL_P_LOC_Cr(0))
#define BPXL_P_YCbCr           0x08000000

typedef enum BPXL_Format
{
	/* RGB */
	BPXL_eA8_R8_G8_B8      = BPXL_P_ALPHA | BPXL_P_COLOR | BPXL_P_RGB | BPXL_P_LOC_ARGB | 0x8888,

	BPXL_eR5_G6_B5         = BPXL_P_COLOR | BPXL_P_RGB | BPXL_P_LOC_ARGB | 0x0565,
	BPXL_eA8_Y8_Cb8_Cr8    = BPXL_P_ALPHA | BPXL_P_COLOR | BPXL_P_YCbCr | BPXL_P_LOC_AYCbCr | 0x8888,

	BPXL_INVALID           = 0
} BPXL_Format;

typedef int BFMT_VideoFmt ;

#define true 1
#define false 0

typedef unsigned int BERR_Code ;

/***************************************************************************
Summary:
	Extracts the bitfield value of a register using RDB name.
**************************************************************************/
#define BCHP_GET_FIELD_DATA(Memory,Register,Field) \
	((((Memory) & BCHP_MASK(Register,Field)) >> \
	BCHP_SHIFT(Register,Field)))

/***************************************************************************
Summary:
	Push a value into a register bitfield using RDB name.
**************************************************************************/

#define BCHP_FIELD_DATA(Register,Field,Data) \
	((Data) << BCHP_SHIFT(Register,Field))

/***************************************************************************
Summary:
	Push a  value name into a register bitfield using RDB name.
**************************************************************************/
#define BCHP_FIELD_ENUM(Register,Field,Name) \
	BCHP_FIELD_DATA(Register,Field, BCHP_##Register##_##Field##_##Name)

/*************************************************************************
Summary:
	Provide a mask for a specific register field.
**************************************************************************/
#define BCHP_MASK(Register,Field) \
	BCHP_##Register##_##Field##_MASK

/*************************************************************************
Summary:
	Provide a shift for a specific register field.
**************************************************************************/
#define BCHP_SHIFT(Register,Field) \
	BCHP_##Register##_##Field##_SHIFT


/* KNI related calls */
#define BKNI_Memcpy(d,s,c)	memcpy(d,s,c)
/* #define BMEM_AllocAligned(Heap, Size, Alignment, Boundry)	BCM_K0_TO_K1( (uint32_t)KMALLOC(Size, (1<<Alignment))) */
void *BMEM_AllocAligned
(
	BMEM_Handle       pheap,       /* Heap to allocate from */
	size_t            ulSize,      /* size in bytes of block to allocate */
	unsigned int      ucAlignBits, /* alignment for the block */
	unsigned int      Boundary     /* boundry restricting allocated value */
);

BERR_Code BMEM_ConvertAddressToOffset(BMEM_Handle heap, void* addr, uint32_t* offset) ;
uint32_t AlignAddress(
		uint32_t	ui32Address,	/* [in] size in bytes of block to allocate */
		unsigned int uiAlignBits	/* [in] alignment for the block */
		);

BERR_Code BMEM_Heap_ConvertAddressToCached(
		BMEM_Handle Heap, 
		void *pvAddress, 
		void **ppvCachedAddress
);

BERR_Code BMEM_Heap_FlushCache(
		BMEM_Handle Heap,
		void *pvCachedAddress,
		size_t size
);

#define BDBG_MODULE(x)

/* #define SPLASH_DEBUG */

#define BDBG_ERR(x) xprintf x
#ifdef SPLASH_DEBUG
#define BDBG_MSG(x) xprintf x
#else
#define BDBG_MSG(x)
#endif

#endif /* __SPLASH_MAGNUM_H__ */

/* End of File */

