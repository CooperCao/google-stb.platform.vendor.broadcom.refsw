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
 * This file contains the platform specific interrupt 
 *   tables and shread functions 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#include <linux/autoconf.h>
#else
#include <linux/config.h>
#endif
#include <linux/module.h>
#include "bcmdriver_common.h"

#define NUM_INTC_WORDS   3   /* Number of interrupt controller words */

 /*
manage: kbd1irq(0),ldkirq(1),irbirq(2),3,4,kbd2irq(5),gioirq(6),icapirq(7),kbd3irq(8)-->  UPG_CPU_INTR
dont manage: uairq, ubirq, ucirq
spiirq spiirq  -->  UPG_SPI_CPU_INTR
iicairq, iicbirq, iiccirq, iicdirq  -->  UPG_BSC_CPU_INTR */
#define BCM7325_UPG_MANAGED_BITS (0x000003e7)

void bcm7325_upg_l2share_func(int irq, unsigned long *triggeredInts)
{
    volatile unsigned long *l2_base = (volatile unsigned long *)0xB0400780; /* UPG_CPU_INTR, shared with kernel */
    unsigned long status, enable;
    
    status = l2_base[1];
    enable = l2_base[0];

    *triggeredInts = status & enable & BCM7325_UPG_MANAGED_BITS;

    /* Mask all triggered interrupts */
    if (*triggeredInts) {
        l2_base[0] = l2_base[0] & ~(*triggeredInts);
    }
}

/* function to mask the managed interrupts */
void bcm7325_upg_l2mask_func(int irq, unsigned long *unused)
{
    volatile unsigned long *l2_base = (volatile unsigned long *)0xB0400780; /* UPG_CPU_INTR, shared with kernel */
    /* Mask all managed interrupts */
    l2_base[0] = l2_base[0] & ~BCM7325_UPG_MANAGED_BITS;
    
}

s_InteruptTable interruptName7325[] = {
    {NULL, 0}, /* Linux IRQ #'s are offset by 1 */
    {"HIF_CPU_INTR",             0}, /* 0 */
    {"XPT_STATUS_CPU_INTR",      1}, /* 1  W0*/
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
    {"MEMC_0_CPU_INTR",          1}, /* 14 */
    {"ENET_EMAC1_CPU_INTR",            0}, /* 15 */
    {"RFM_CPU_INTR",             1}, /* 16 */
    {"UPG_TMR_CPU_INTR",         1}, /* 17 */
     {"UPG_CPU_INTR",             1, 0, bcm7325_upg_l2share_func,bcm7325_upg_l2mask_func}, /* 18 */
    {"UPG_BSC_CPU_INTR",         1}, /* 19 */
    {"UPG_SPI_CPU_INTR",         1}, /* 20 */
    {"UPG_UART0_CPU_INTR",      0}, /* 21 */
    {"UPG_SC_CPU_INTR",          1}, /* 22 */
    {"SUN_CPU_INTR",             1}, /* 23 */
    {"UHF1_CPU_INTR",            1}, /* 24 */
    {NULL,                       0}, /* 25 */
    {"USB_CPU_INTR",             0}, /* 26 */
    {"MC_CPU_INTR",              1}, /* 27 */
    {"BVNF_CPU_INTR_3",          1}, /* 28 */
    {"FTM_CPU_INTR",       1}, /* 29 */
    {"AVD0_CPU_INTR_0",          1}, /* 30 */
    {"XPT_RAVE_CPU_INTR",        1}, /* 31 */
    {"PCI_INTA_0_CPU_INTR",      0 | INT_REG_WRAP}, /* 0 W1*/
    {"PCI_INTA_1_CPU_INTR",      0}, /* 1 */
    {"PCI_INTA_2_CPU_INTR",      0}, /* 2 */
    {"QPSK_RCVR_0_CPU_INTR",                       1}, /* 3 */
    {"EXT_IRQ_0_CPU_INTR",       0}, /* 4 */
    {"EXT_IRQ_1_CPU_INTR",       0}, /* 5 */
    {"EXT_IRQ_2_CPU_INTR",       0}, /* 6, SLOT 2 */
    {"EXT_IRQ_3_CPU_INTR",       0}, /* 7 */
    {"EXT_IRQ_4_CPU_INTR",       0}, /* 8, SLOT 0 */
    {NULL ,        0}, /* 9 Reserved  */
    {"EXT_IRQ_5_CPU_INTR",       1}, /* 10 */
    {"EXT_IRQ_6_CPU_INTR",       0}, /* 11 */
    {"EXT_IRQ_7_CPU_INTR",       1}, /* 12, SLOT 2 */
    {"EXT_IRQ_8_CPU_INTR",       0}, /* 13 */
    {"EXT_IRQ_9_CPU_INTR",       0}, /* 14 */
    {"EXT_IRQ_10_CPU_INTR",      0}, /* 15 */
    {"EXT_IRQ_11_CPU_INTR",      1}, /* 16 */
    {"EXT_IRQ_12_CPU_INTR",      1}, /* 17 */
    {"EXT_IRQ_13_CPU_INTR",      0}, /* 18   */
    {"EXT_IRQ_14_CPU_INTR",      0}, /* 19  */
    {"IPI0_CPI_INTR",            1}, /* 20 */
    {"IPI1_CPU_INTR",            1}, /* 21 */
    {"XPT_MSG_STAT_CPU_INTR", 1 }, /* 22 */
    {"BVNF_CPU_INTR_6",          1}, /* 23 */
    {"AFEC_CORE_CPU_INTR",      1}, /* 24  TODO*/
    {"SM_CPU_INTR",              0}, /* 25 */
    {"USB_OHCI_1_CPU_INTR",      0},  /* 26 */
    {"XPT_PCR_CPU_INTR",         1}, /* 27 */
    {"XPT_FE_CPU_INTR",    1}, /* 28 */
    {"USB_EHCI_0_CPU_INTR",        0}, /* 29 */
    {"USB_EHCI_1_CPU_INTR",      0}, /* 30 */
    {"USB_OHCI_0_CPU_INTR",      0},  /* 31 */
    {"SDS_RCVR_0_CPU_INTR",      1 | INT_REG_WRAP}, /* 0 W2*/
    {"UPG_UART1_CPU_INTR",      0}, /* 1 */
    {"UPG_UART2_CPU_INTR",      0}, /* 2 */
    {"MEMC_1_CPU_INTR",      1}, /* 3 */
    {"SDS_RCVR_1_CPU_INTR",      1}, /* 4 */
    {"QPSK_RCVR_1_CPU_INTR",      1}, /* 5 */
    {NULL ,        0}, /* 6 Reserved  */
    {NULL ,        0}, /* 7 Reserved  */
    {NULL ,        0}, /* 8 Reserved  */
    {NULL ,        0}, /* 9 Reserved  */
    {NULL ,        0}, /* 10 Reserved  */
    {NULL ,        0}, /* 11 Reserved  */
    {NULL ,        0}, /* 12 Reserved  */
    {NULL ,        0}, /* 13 Reserved  */
    {NULL ,        0}, /* 14 Reserved  */
    {NULL ,        0}, /* 15 Reserved  */
    {NULL ,        0}, /* 16 Reserved  */
    {NULL ,        0}, /* 17 Reserved  */
    {NULL ,        0}, /* 18 Reserved  */
    {NULL ,        0}, /* 19 Reserved  */
    {NULL ,        0}, /* 20 Reserved  */
    {NULL ,        0}, /* 21 Reserved  */
    {NULL ,        0}, /* 22 Reserved  */
    {NULL ,        0}, /* 23 Reserved  */
    {NULL ,        0}, /* 24 Reserved  */
    {NULL ,        0}, /* 25 Reserved  */
    {NULL ,        0}, /* 26 Reserved  */
    {NULL ,        0}, /* 27 Reserved  */
    {NULL ,        0}, /* 28 Reserved  */
    {NULL ,        0}, /* 29 Reserved  */
    {NULL ,        0}, /* 30 Reserved  */
    {NULL ,        0} /* 31 Reserved  */
};


s_ChipConfig  g_sChipConfig ={ "7325",interruptName7325, 0xb0001400, sizeof(interruptName7325)/sizeof(s_InteruptTable),NUM_INTC_WORDS};
