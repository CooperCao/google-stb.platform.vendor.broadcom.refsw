/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
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
    ((uint64_t)(Data) << BCHP_SHIFT(Register,Field))

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
#define BKNI_Memcpy(d,s,c)  memcpy(d,s,c)

typedef BSTD_DeviceOffset BMMA_DeviceOffset;
typedef void* BMMA_HEAP_Handle;
typedef void* BMMA_Block_Handle;

BMMA_Block_Handle BMMA_Alloc
(
    BMMA_Heap_Handle heap,              /* Heap to allocate from */
    size_t size,                        /* Size in bytes of block to allocate */
    unsigned alignment,                 /* Size in bytes of block to allocate */
    const BMMA_AllocationSettings *pSettings    /* Unused */
);

void* BMMA_Lock
(
    BMMA_Block_Handle block
);

BMMA_DeviceOffset BMMA_LockOffset
(
    BMMA_Block_Handle block
);

void BMMA_Unlock
(
    BMMA_Block_Handle block,
    const void *addr
);

void BMMA_UnlockOffset
(
    BMMA_Block_Handle block,
    BMMA_DeviceOffset offset
);

void BMMA_FlushCache
(
    BMMA_Block_Handle block,
    const void *addr,
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
