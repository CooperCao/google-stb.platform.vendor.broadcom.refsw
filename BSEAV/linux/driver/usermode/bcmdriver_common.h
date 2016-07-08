/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef BCMDRIVER_COMMON_H
#define BCMDRIVER_COMMON_H

/**
Summary:
BCHP_VER_XX values are used with the BCHP_VER and BCHP_<<secondary_chip>>_VER macros
to provide version-specific software support.

Description:
Certain chip revisions within a chip family have different interrupt bitfield definitions. As such
these need to be distinguished using BCHP_VER.

**/

#define BCHP_VER_A0 (0x00000000)
#define BCHP_VER_A1 (0x00000001)
#define BCHP_VER_A2 (0x00000002)
#define BCHP_VER_A3 (0x00000003)
#define BCHP_VER_A4 (0x00000004)
#define BCHP_VER_A5 (0x00000005)

#define BCHP_VER_B0 (0x00010000)
#define BCHP_VER_B1 (0x00010001)
#define BCHP_VER_B2 (0x00010002)
#define BCHP_VER_B3 (0x00010003)
#define BCHP_VER_B4 (0x00010004)
#define BCHP_VER_B5 (0x00010005)

#define BCHP_VER_C0 (0x00020000)
#define BCHP_VER_C1 (0x00020001)
#define BCHP_VER_C2 (0x00020002)
#define BCHP_VER_C3 (0x00020003)
#define BCHP_VER_C4 (0x00020004)
#define BCHP_VER_C5 (0x00020005)

#define BCHP_VER_D0 (0x00030000)
#define BCHP_VER_D1 (0x00030001)
#define BCHP_VER_D2 (0x00030002)
#define BCHP_VER_D3 (0x00030003)
#define BCHP_VER_D4 (0x00030004)
#define BCHP_VER_D5 (0x00030005)

#define BCHP_VER_E0 (0x00040000)
#define BCHP_VER_E1 (0x00040001)
#define BCHP_VER_E2 (0x00040002)
#define BCHP_VER_E3 (0x00040003)
#define BCHP_VER_E4 (0x00040004)
#define BCHP_VER_E5 (0x00040005)

/* unused - now assuming that linux irqs have straight mapping to L1 bits */
#define INT_REG_WRAP 0 

#define INT_ENABLE_MASK 0x01
#define INT_SHARABLE_MASK 0x02
#define INT_SHARABLE INT_SHARABLE_MASK
#define INT_VIRTUAL_MASK    0x04

/* See sIntName below for how this is used. The irq param might be ignored if a separate function is used for each special L2.
The function must return the correct triggeredInts after reading status, applying the mask, masking out ints used by others.
This function is responsible for disabling the L2 as well. */
typedef void (*l2share_func_t)(int irq, unsigned long *triggeredInts);

typedef struct s_InteruptTable
{
    const char *name;               /* Text name of interrupt */
    int manageInt;                  /* 0000.0000 - do not mananage this interrupt, x000.0001 - manage this interrupt,  x000.001x = sharable, 1000.0000 = wrapped */
    unsigned long numInter;         /* number of interrupts */
    l2share_func_t l2share_func;    /* if set, then process L2 register with special code. this allows L2's to be masked in the driver.
                                       this is necessary if other L2's (which are not managed by the usermodedrv) may remain set, causing endless L1 interrupts. */
    l2share_func_t mask_func;       /* function to mask the managed bits */
    int disabled;
}s_InteruptTable;

typedef struct s_ChipConfig
{
    const char *chipName;
    s_InteruptTable *pIntTable;
    unsigned long intcAddr; /* unused */
    unsigned int  maxNumIrq; /* total number of irqs, including 0. so, this is exclusive. */
    unsigned int  IntcSize;         /* Interrupt controller size in words */
}s_ChipConfig;

extern s_ChipConfig  g_sChipConfig;   /* defined in interrupt_table.c file of each chip directory */

#endif /* BCMDRIVER_COMMON_H */
