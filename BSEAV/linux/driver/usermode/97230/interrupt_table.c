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
 * This file contains the platform specific interrupt tables 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37) 
#elif ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) ) && ( LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37) )
#include <linux/autoconf.h>
#else
#include <linux/config.h>
#endif
#include <linux/module.h>
#include "bcmdriver_common.h"
    

#define NUM_INTC_WORDS   3   /* Number of interrupt controller words */


s_InteruptTable interruptName7231[] = {
    {NULL, 0},                   /* Linux IRQ #'s are offset by 1 */
    {"AIO_CPU_INTR",             INT_ENABLE_MASK},/* 0 */
    {"BSP_CPU_INTR",             INT_ENABLE_MASK},/* 1 */
    {"BVNB_0_CPU_INTR",          INT_ENABLE_MASK},/* 2 */
    {"BVNF_0_CPU_INTR",          INT_ENABLE_MASK},/* 3 */
    {"BVNF_1_CPU_INTR",          INT_ENABLE_MASK},/* 4 */
    {"BVNF_5_CPU_INTR",          INT_ENABLE_MASK},/* 5 */
    {"BVNM_0_CPU_INTR",          INT_ENABLE_MASK},/* 6 */
    {"CLKGEN_CPU_INTR",          INT_ENABLE_MASK},/* 7 */
    {"EXT_IRQ_00_CPU_INTR",      INT_ENABLE_MASK},/* 8 */
    {"EXT_IRQ_01_CPU_INTR",      INT_ENABLE_MASK},/* 9 */
    {"EXT_IRQ_02_CPU_INTR",      INT_ENABLE_MASK},/* 10 */
    {"EXT_IRQ_03_CPU_INTR",      INT_ENABLE_MASK},/* 11 */
    {"EXT_IRQ_04_CPU_INTR",      INT_ENABLE_MASK},/* 12 */
    {"EXT_IRQ_05_CPU_INTR",      0}, /* 13 */
    {"EXT_IRQ_06_CPU_INTR",      0}, /* 14 */
    {"EXT_IRQ_07_CPU_INTR",      0}, /* 15 */
    {"EXT_IRQ_08_CPU_INTR",      0}, /* 16 */
    {"EXT_IRQ_09_CPU_INTR",      0}, /* 17 */
    {"EXT_IRQ_10_CPU_INTR",      0}, /* 18 */
    {"EXT_IRQ_11_CPU_INTR",      0}, /* 19 */
    {"EXT_IRQ_12_CPU_INTR",      0}, /* 20 */
    {"EXT_IRQ_13_CPU_INTR",      0}, /* 21 */
    {"EXT_IRQ_14_CPU_INTR",      0}, /* 22 */
    {"FTM_CPU_INTR",             0}, /* 23 */
    {"GENET_0_A_CPU_INTR",       0}, /* 24 */
    {"GENET_0_B_CPU_INTR",       0}, /* 25 */
    {"GENET_1_A_CPU_INTR",       0}, /* 26 */
    {"GENET_1_B_CPU_INTR",       0}, /* 27 */
    {"GFX_CPU_INTR",             INT_ENABLE_MASK}, /* 28 */
    {"HDMI_TX_CPU_INTR",         INT_ENABLE_MASK}, /* 29 */
    {"HIF_CPU_INTR",             0}, /* 30 */
    {"HIF_SPI_CPU_INTR",         0}, /* 31 */
    {"IPI0_CPU_INTR",            INT_ENABLE_MASK| INT_REG_WRAP}, /* 0 */
    {"IPI1_CPU_INTR",            INT_ENABLE_MASK}, /* 1 */
    {"MEMC0_CPU_INTR",           INT_ENABLE_MASK}, /* 2 */
    {"MOCA_CPU_INTR",            0}, /* 3 */
    {"NMI_PIN_CPU_INTR",         INT_ENABLE_MASK}, /* 4 */
    {"RAAGA_CPU_INTR",           INT_ENABLE_MASK}, /* 5 */
    {"RAAGA_FW_CPU_INTR",        INT_ENABLE_MASK}, /* 6 */
    {"SATA_GRB_CPU_INTR",        0}, /* 7 */
    {"SATA_AHCI_CPU_INTR",       0}, /* 8 */
    {"SDS0_AFEC_CPU_INTR",       0}, /* 9 */
    {"SDS0_RCVR_0_CPU_INTR",     0}, /* 10 */
    {"SDS0_RCVR_1_CPU_INTR",     0}, /* 11 */
    {"SDS0_TFEC_CPU_INTR",       0}, /* 12 */
    {"SDS1_AFEC_CPU_INTR",       0 }, /* 13 */
    {"SDS1_RCVR_0_CPU_INTR",     0}, /* 14 */
    {"SDS1_RCVR_1_CPU_INTR",     0}, /* 15 */
    {"SDS1_TFEC_CPU_INTR",       0}, /* 16 */
    {"SOFT_MODEM_CPU_INTR",      INT_ENABLE_MASK}, /* 17 */
    {"SVD0_0_CPU_INTR",          INT_ENABLE_MASK}, /* 18 */
    {"SYS_CPU_INTR",             INT_ENABLE_MASK}, /* 19 */
    {"SYS_AON_CPU_INTR",         INT_ENABLE_MASK}, /* 20 */
    {"SYS_PM_CPU_INTR",          0}, /* 21 */
    {"UHF_CPU_INTR",             0}, /* 22 */
    {"UPG_AUX_CPU_INTR",         INT_ENABLE_MASK}, /* 23 */
    {"UPG_AUX_AON_CPU_INTR",     INT_ENABLE_MASK}, /* 24 */
    {"UPG_BSC_CPU_INTR",         INT_ENABLE_MASK}, /* 25 */
    {"UPG_BSC_AON_CPU_INTR",     INT_ENABLE_MASK}, /* 26 */
    {"UPG_MAIN_CPU_INTR",        INT_ENABLE_MASK}, /* 27 */
    {"UPG_MAIN_AON_CPU_INTR",    INT_ENABLE_MASK}, /* 28 */
    {"UPG_SC_CPU_INTR",          INT_ENABLE_MASK}, /* 29 */
    {"UPG_SPI_CPU_INTR",         INT_ENABLE_MASK}, /* 30 */
    {"UPG_TMR_CPU_INTR",         INT_ENABLE_MASK}, /* 31 */
    {"UPG_UART0_CPU_INTR",       0 | INT_REG_WRAP }, /* 0 */
    {"UPG_UART1_CPU_INTR",       0}, /* 1 */
    {"UPG_UART2_CPU_INTR",       0},  /* 2 */
    {"USB0_BRIDGE_CPU_INTR",     0}, /* 3 */
    {"USB0_EHCI_0_CPU_INTR",     0}, /* 4 */
    {"USB0_EHCI_1_CPU_INTR",     0}, /* 5 */
    {"USB0_OHCI_0_CPU_INTR",     0}, /* 6 */
    {"USB0_OHCI_1_CPU_INTR",     0}, /* 7 */
    {"USB1_BRIDGE_CPU_INTR",     0}, /* 8 */
    {"USB1_EHCI_0_CPU_INTR",     0}, /* 9 */
    {"USB1_EHCI_1_CPU_INTR ",    0}, /* 10 */
    {"USB1_OHCI_0_CPU_INTR",     0}, /* 11 */
    {"USB1_OHCI_1_CPU_INTR",     0}, /* 12 */
    {"VEC_CPU_INTR",             INT_ENABLE_MASK}, /* 13 */
    {"XPT_FE_CPU_INTR",          INT_ENABLE_MASK}, /* 14 */
    {"XPT_MSG_CPU_INTR",         INT_ENABLE_MASK}, /* 15 */
    {"XPT_MSG_STAT_CPU_INTR",    INT_ENABLE_MASK}, /* 16 */
    {"XPT_OVFL_CPU_INTR",        INT_ENABLE_MASK}, /* 17 */
    {"XPT_PCR_CPU_INTR",         INT_ENABLE_MASK}, /* 18 */
    {"XPT_RAV_CPU_INTR",         INT_ENABLE_MASK}, /* 19 */
    {"XPT_STATUS_CPU_INTR",      INT_ENABLE_MASK}, /* 20 */
    {NULL,                       0}, /* 21 */
    {NULL,                       0}, /* 22 */
    {NULL,                       0}, /* 23 */
    {NULL,                       0}, /* 24 */
    {NULL,                       0}, /* 25 */
    {NULL,                       0}, /* 26 */
    {NULL,                       0}, /* 27 */
    {NULL,                       0}, /* 28 */
    {NULL,                       0}, /* 29 */
    {NULL,                       0}, /* 30 */
    {NULL,                       0}  /* 31 */
};


s_ChipConfig  g_sChipConfig ={ "7231",interruptName7231, 0xb0411400, sizeof(interruptName7231)/sizeof(s_InteruptTable),NUM_INTC_WORDS};
