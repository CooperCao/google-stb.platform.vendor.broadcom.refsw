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


s_InteruptTable interruptName7429[] = {
    {NULL, 0},                   /* Linux IRQ #'s are offset by 1 */
    {"AIO_CPU_INTR",             INT_ENABLE_MASK},/* 0 */
    {"BSP_CPU_INTR",             INT_ENABLE_MASK},/* 1 */
    {"BVNB_0_CPU_INTR",          INT_ENABLE_MASK},/* 2 */
    {"BVNF_0_CPU_INTR",          INT_ENABLE_MASK},/* 3 */
    {"BVNF_1_CPU_INTR",          INT_ENABLE_MASK},/* 4 */
    {"BVNF_5_CPU_INTR",          INT_ENABLE_MASK},/* 5 */
    {"BVNM_0_CPU_INTR",          INT_ENABLE_MASK},/* 6 */
    {"CLKGEN_CPU_INTR",          INT_ENABLE_MASK},/* 7 */
    {"DVP_HR_CPU_INTR",          INT_ENABLE_MASK},/* 8 */
    {"EXT_IRQ_00_CPU_INTR",      INT_ENABLE_MASK},/* 9 */
    {"EXT_IRQ_01_CPU_INTR",      INT_ENABLE_MASK},/* 10 */
    {"EXT_IRQ_02_CPU_INTR",      INT_ENABLE_MASK},/* 11 */
    {"EXT_IRQ_03_CPU_INTR",      INT_ENABLE_MASK},/* 12 */
    {"EXT_IRQ_04_CPU_INTR",      INT_ENABLE_MASK}, /* 13 */
    {"EXT_IRQ_05_CPU_INTR",      INT_ENABLE_MASK}, /* 14 */
    {"GENET_0_A_CPU_INTR",       0}, /* 15 */
    {"GENET_0_B_CPU_INTR",       0}, /* 16 */
    {"GENET_1_A_CPU_INTR",       0}, /* 17 */
    {"GENET_1_B_CPU_INTR",       0}, /* 18 */
    {"GFX_CPU_INTR",             INT_ENABLE_MASK}, /* 19 */
    {"HDMI_RX_0_CPU_INTR",       INT_ENABLE_MASK}, /* 20 */
    {"HDMI_TX_CPU_INTR",         INT_ENABLE_MASK}, /* 21 */
    {"HIF_CPU_INTR",             0}, /* 22 */
    {"HIF_SPI_CPU_INTR",         0}, /* 23 */
    {"IPI0_CPU_INTR",            INT_ENABLE_MASK}, /* 24 */
    {"IPI1_CPU_INTR",            INT_ENABLE_MASK}, /* 25 */
    {"MEMC0_CPU_INTR",           INT_ENABLE_MASK}, /* 26 */
    {"MOCA_CPU_INTR",            0}, /* 27 */
    {"NMI_PIN_CPU_INTR",         INT_ENABLE_MASK}, /* 28 */
    {"RAAGA_CPU_INTR",           INT_ENABLE_MASK}, /* 29 */
    {"RAAGA_FW_CPU_INTR",        INT_ENABLE_MASK}, /* 30 */
    {"RFM_CPU_INTR",             INT_ENABLE_MASK}, /* 31 */
    {"SATA_AHCI_CPU_INTR",       0| INT_REG_WRAP}, /* 0 */
    {"SATA_GRB_CPU_INTR",        0}, /* 1 */
    {"SDIO0_0_CPU_INTR",         0}, /* 2 */
    {"SDIO1_0_CPU_INTR",         0}, /* 3 */
    {"SOFT_MODEM_CPU_INTR",      INT_ENABLE_MASK}, /* 4 */
    {"SVD0_0_CPU_INTR",          INT_ENABLE_MASK}, /* 5 */
    {"SYS_CPU_INTR",             INT_ENABLE_MASK}, /* 6 */
    {"SYS_AON_CPU_INTR",         INT_ENABLE_MASK}, /* 7 */
    {"SYS_PM_CPU_INTR",          0}, /* 8 */
    {"UPG_AUX_CPU_INTR",         INT_ENABLE_MASK}, /* 9 */
    {"UPG_AUX_AON_CPU_INTR",     INT_ENABLE_MASK}, /* 10 */
    {"UPG_BSC_CPU_INTR",         INT_ENABLE_MASK}, /* 11 */
    {"UPG_BSC_AON_CPU_INTR",     INT_ENABLE_MASK}, /* 12 */
    {"UPG_MAIN_CPU_INTR",        INT_ENABLE_MASK }, /* 13 */
    {"UPG_MAIN_AON_CPU_INTR",    INT_ENABLE_MASK}, /* 14 */
    {"UPG_MC_CPU_INTR",          0}, /* 15 */
    {"UPG_SC_CPU_INTR",          INT_ENABLE_MASK}, /* 16 */
    {"UPG_SPI_CPU_INTR",         INT_ENABLE_MASK}, /* 17 */
    {"UPG_TMR_CPU_INTR",         INT_ENABLE_MASK}, /* 18 */
    {"UPG_UART0_CPU_INTR",       0}, /* 19 */
    {"UPG_UART1_CPU_INTR",       0}, /* 20 */
    {"UPG_UART2_CPU_INTR",       0}, /* 21 */
    {"USB0_BRIDGE_CPU_INTR",     0}, /* 22 */
    {"USB0_EHCI_0_CPU_INTR",     0}, /* 23 */
    {"USB0_EHCI_1_CPU_INTR",     0}, /* 24 */
    {"USB0_OHCI_0_CPU_INTR",     0}, /* 25 */
    {"USB0_OHCI_1_CPU_INTR",     0}, /* 26 */
    {"USB1_BRIDGE_CPU_INTR",     0}, /* 27 */
    {"USB1_EHCI_0_CPU_INTR",     0}, /* 28 */
    {"SB1_EHCI_1_CPU_INTR",      0}, /* 29 */
    {"USB1_OHCI_0_CPU_INTR",     0}, /* 30 */
    {"USB1_OHCI_1_CPU_INTR",     0}, /* 31 */
    {"VEC_CPU_INTR",             INT_ENABLE_MASK | INT_REG_WRAP }, /* 0 */
    {"XPT_FE_CPU_INTR",          INT_ENABLE_MASK}, /* 1 */
    {"XPT_MSG_CPU_INTR",         INT_ENABLE_MASK},  /* 2 */
    {"XPT_MSG_STAT_CPU_INTR",    INT_ENABLE_MASK}, /* 3 */
    {"XPT_OVFL_CPU_INTR",        INT_ENABLE_MASK}, /* 4 */
    {"XPT_PCR_CPU_INTR",         INT_ENABLE_MASK}, /* 5 */
    {"XPT_RAV_CPU_INTR",         INT_ENABLE_MASK}, /* 6 */
    {"XPT_STATUS_CPU_INTR",      INT_ENABLE_MASK}, /* 7 */
    {NULL,                       0}, /* 8 */
    {NULL,                       0}, /* 9 */
    {NULL,                       0}, /* 10 */
    {NULL,                       0}, /* 11 */
    {NULL,                       0}, /* 12 */
    {NULL,                       0}, /* 13 */
    {NULL,                       0}, /* 14 */
    {NULL,                       0}, /* 15 */
    {NULL,                       0}, /* 16 */
    {NULL,                       0}, /* 17 */
    {NULL,                       0}, /* 18 */
    {NULL,                       0}, /* 19 */
    {NULL,                       0}, /* 20 */
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


s_ChipConfig  g_sChipConfig ={ "7429",interruptName7429, 0xb0411400, sizeof(interruptName7429)/sizeof(s_InteruptTable),NUM_INTC_WORDS};
