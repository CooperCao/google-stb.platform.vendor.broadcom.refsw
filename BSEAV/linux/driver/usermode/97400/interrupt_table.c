/***************************************************************************
 *     Copyright (c) 2002-2008, Broadcom Corporation
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
 * This file contains the platform specific interrupt tables 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <linux/config.h>
#include <linux/module.h>
#include "bcmdriver_common.h"

#define NUM_INTC_WORDS   2   /* Number of interrupt controller words */

/*  
    manage: kbd1irq(0), kbd2irqi(5), kbd3irq(8), kbd4irq(10), ldkirq(1), irbirq(2), gioirq(6), icapirq(7)
    don't manage: uairq(4), ubirq(3), ucirq(9), spi(20) iica..e(24..28).
*/
#define BCM7400_UPG_MANAGED_BITS (0x000005e7)

void bcm7400_upg_l2share_func(int irq, unsigned long *triggeredInts)
{
    volatile unsigned long *l2_base = (volatile unsigned long *)0xB0400780; /* UPG_CPU_INTR, shared with kernel */
    unsigned long status, enable;
    
    status = l2_base[1];
    enable = l2_base[0];

    *triggeredInts = status & enable & BCM7400_UPG_MANAGED_BITS;

    /* Mask all triggered interrupts */
    if (*triggeredInts) {
        l2_base[0] = l2_base[0] & ~(*triggeredInts);
    }
}

/* function to mask the managed interrupts */
void bcm7400_upg_l2mask_func(int irq, unsigned long *unused)
{
    volatile unsigned long *l2_base = (volatile unsigned long *)0xB0400780; /* UPG_CPU_INTR, shared with kernel */
    /* Mask all managed interrupts */
    l2_base[0] = l2_base[0] & ~BCM7400_UPG_MANAGED_BITS;
    
}

s_InteruptTable interruptName7400b0[] = {
    {NULL, 0}, /* Linux IRQ #'s are offset by 1 */
    {"HIF_CPU_INTR",             1}, /* 0 */
    {"XPT_STATUS_CPU_INTR",      1}, /* 1 */
    {"XPT_OVFL_CPU_INTR",        1}, /* 2 */
    {"XPT_MSG_CPU_INTR",         1}, /* 3 */
    {"XPT_ICAM_CPU_INTR",        1}, /* 4 */
    {"BSP_CPU_INTR",             1}, /* 5 */
    {"AIO_CPU_INTR",             1}, /* 6 */
    {"GFX_CPU_INTR",             1}, /* 7 */
    {"HDMI_CPU_INTR",            1}, /* 8 */
    {"RPTD_CPU_INTR",            1}, /* 9 */
    {"VEC_CPU_INTR",             1}, /* 10 */
    {"BVNB_CPU_INTR",            1}, /* 11 */
    {"BVNF_CPU_INTR_0",          1}, /* 12 */
    {"BVNF_CPU_INTR_1",          1}, /* 13 */
    {"BVNF_CPU_INTR_2",          1}, /* 14 */
    {"ENET_CPU_INTR",            0}, /* 15 */
    {"RFM_CPU_INTR",             1}, /* 16 */
    {"UPG_TMR_CPU_INTR",         1}, /* 17 */
    {"UPG_CPU_INTR",             1, 0, bcm7400_upg_l2share_func,bcm7400_upg_l2mask_func}, /* 18 */
    {"UPG_BSC_CPU_INTR",         1}, /* 19 */
    {"UPG_SPI_CPU_INTR",         1}, /* 20 */
    {"UPG_UART0_CPU_INTR",       0}, /* 21 */
    {"UPG_SC_CPU_INTR",          1}, /* 22 */
    {"SUN_CPU_INTR",             1}, /* 23 */
    {"UHF1_CPU_INTR",            1}, /* 24 */
    {"UHF2_CPU_INTR",            1}, /* 25 */
    {"USB_CPU_INTR",             0}, /* 26 */
    {"MC_CPU_INTR",              0}, /* 27 */
    {"BVNF_CPU_INTR_3",          1}, /* 28 */
    {"SATA_PCIB_CPU_INTR",       0}, /* 29 */
    {"AVD0_CPU_INTR_0",          1}, /* 30 */
    {"XPT_RAVE_CPU_INTR",        1}, /* 31 */
    {"PCI_INTA_0_CPU_INTR",      1 | INT_REG_WRAP}, /* 0 */
    {"PCI_INTA_1_CPU_INTR",      1}, /* 1 */
    {"PCI_INTA_2_CPU_INTR",      0}, /* 2 */
    {"PX3D_CPU_INTR",            1}, /* 3 */
    {"EXT_IRQ_0_CPU_INTR",       1}, /* 4 */
    {"EXT_IRQ_1_CPU_INTR",       1}, /* 5, 3517 */
    {"EXT_IRQ_2_CPU_INTR",       0}, /* 6 */
    {"EXT_IRQ_3_CPU_INTR",       1}, /* 7 */
    {"EXT_IRQ_4_CPU_INTR",       1}, /* 8, Slot 2 */
    {"PCI_SATA_PCI_INTR",        0}, /* 9 */
    {"EXT_IRQ_5_CPU_INTR",       0}, /* 10 */
    {"EXT_IRQ_6_CPU_INTR",       1}, /* 11, 4500 */
    {"EXT_IRQ_7_CPU_INTR",       0}, /* 12 */
    {"EXT_IRQ_8_CPU_INTR",       0}, /* 13 */
    {"EXT_IRQ_9_CPU_INTR",       0}, /* 14 */
    {"EXT_IRQ_10_CPU_INTR",      0}, /* 15 */
    {"EXT_IRQ_11_CPU_INTR",      1}, /* 16, Slot 0a */
    {"EXT_IRQ_12_CPU_INTR",      0}, /* 17 */
    {"EXT_IRQ_13_CPU_INTR",      1}, /* 18, Slot 1a */
    {"EXT_IRQ_14_CPU_INTR",      0}, /* 19 */
    {"IPI0_CPI_INTR",            1}, /* 20 */
    {"IPI1_CPU_INTR",            1}, /* 21 */
    {"AVD1_CPU_INTR",            1}, /* 22 */
    {"BVNF_CPU_INTR_6",          1}, /* 23 */
    {"USB_EHCI_1_CPU_INTR",      0}, /* 24 */
    {"SM_CPU_INTR",              0}, /* 25 */
    {"XPT_PCR_CPU_INTR",         1}, /* 26 */
    {"XPT_FE_CPU_INTR",          1}, /* 27 */
    {"XPT_MSG_STAT_CPU_INTR",    1}, /* 28 */
    {"USB_EHCI_CPU_INTR",        0}, /* 29 */
    {"USB_OHCI_0_CPU_INTR",      0}, /* 30 */
    {"USB_OHCI_1_CPU_INTR",      0}  /* 31 */
};

s_ChipConfig  g_sChipConfig ={ "7400",interruptName7400b0, 0xb0001400, sizeof(interruptName7400b0)/sizeof(s_InteruptTable),NUM_INTC_WORDS};
