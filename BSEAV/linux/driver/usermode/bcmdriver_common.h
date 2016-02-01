/***************************************************************************
 *     Copyright (c) 2002-2012, Broadcom Corporation
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
