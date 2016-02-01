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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#include <generated/autoconf.h>
#else
#include <linux/config.h>
#endif
#include <linux/module.h>
#include "bcmdriver_common.h"
    


#define NUM_INTC_WORDS   3   /* Number of interrupt controller words */


s_InteruptTable interruptName7358[] = {
    {NULL, 0},                   /* Linux IRQ #'s are offset by 1 */
    {"AIO_CPU_INTR",             INT_ENABLE_MASK},/* 0 */
    {"BSP_CPU_INTR",        	 INT_ENABLE_MASK},/* 1 */
    {"BVNB_0_CPU_INTR",        	 INT_ENABLE_MASK},/* 2 */
    {"BVNF_0_CPU_INTR",          INT_ENABLE_MASK},/* 3 */
    {"BVNF_1_CPU_INTR",       	 INT_ENABLE_MASK},/* 4 */
    {"BVNF_5_CPU_INTR",          INT_ENABLE_MASK},/* 5 */
    {"BVNM_0_CPU_INTR",          INT_ENABLE_MASK},/* 6 */
    {"CLKGEN_CPU_INTR",          INT_ENABLE_MASK},/* 7 */
    /* TODO: EXT_IRQ_xx */
    {"EXT_IRQ_00_CPU_INTR",      0},/* 8 */
    {"EXT_IRQ_01_CPU_INTR",      INT_ENABLE_MASK},/* 9 */
    {"EXT_IRQ_02_CPU_INTR",      0},/* 10 */
    {"EXT_IRQ_03_CPU_INTR",      0},/* 11 */
    {"EXT_IRQ_04_CPU_INTR",      0},/* 12 */
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
    {"FTM_CPU_INTR",          	 INT_ENABLE_MASK}, /* 23 */
    {"GENET_0_A_CPU_INTR",       0}, /* 24 */
    {"GENET_0_B_CPU_INTR",       0}, /* 25 */
    {"SPARE_00_CPU_INTR",        0}, /* 26 */    
    {"SPARE_01_CPU_INTR",        0}, /* 27 */    
    {"M2MC_CPU_INTR",            INT_ENABLE_MASK}, /* 28 */
    {"HDMI_TX_CPU_INTR",         INT_ENABLE_MASK}, /* 29 */
    {"HIF_CPU_INTR",             0}, /* 30 */
    {"HIF_SPI_CPU_INTR",         INT_ENABLE_MASK}, /* 31 */
    {"IPI0_CPU_INTR",            INT_ENABLE_MASK | INT_REG_WRAP}, /* 0 */
    {"IPI1_CPU_INTR",            INT_ENABLE_MASK}, /* 1 */
    {"MEMC0_CPU_INTR",           INT_ENABLE_MASK}, /* 2 */
    {"SPARE_02_CPU_INTR",        0}, /* 3 */    
    {"NMI_PIN_CPU_INTR",         INT_ENABLE_MASK}, /* 4 */
    {"RAAGA_CPU_INTR",       	 INT_ENABLE_MASK}, /* 5 */
    {"RAAGA_FW_CPU_INTR",        INT_ENABLE_MASK}, /* 6 */
    {"SDS0_AFEC_CPU_INTR",       INT_ENABLE_MASK}, /* 7 */
    {"SDS0_RCVR_0_CPU_INTR",     INT_ENABLE_MASK}, /* 8 */
    {"SDS0_RCVR_1_CPU_INTR",     INT_ENABLE_MASK}, /* 9 */
    {"SDS0_TFEC_CPU_INTR",       INT_ENABLE_MASK}, /* 10 */
    {"SPARE_03_CPU_INTR",        0}, /* 11 */    
    {"SPARE_04_CPU_INTR",        0}, /* 12 */    
    {"SPARE_05_CPU_INTR",        0}, /* 13 */    
    {"SPARE_06_CPU_INTR",        0}, /* 14 */    
    {"SOFT_MODEM_CPU_INTR",      INT_ENABLE_MASK}, /* 15 */
    {"SYS_CPU_INTR",       		 INT_ENABLE_MASK}, /* 16 */
    {"SYS_AON_CPU_INTR",         INT_ENABLE_MASK}, /* 17 */
    {"SYS_PM_CPU_INTR",       	 0}, /* 18 */
    {"SPARE_07_CPU_INTR",        0}, /* 19 */    
    {"SPARE_08_CPU_INTR",        0}, /* 20 */    
    {"UPG_AUX_AON_CPU_INTR",     INT_ENABLE_MASK}, /* 21 */
    {"UPG_BSC_CPU_INTR",         INT_ENABLE_MASK}, /* 22 */
    {"UPG_BSC_AON_CPU_INTR",     INT_ENABLE_MASK}, /* 23 */
    {"UPG_MAIN_CPU_INTR",      	 INT_ENABLE_MASK}, /* 24 */
    {"UPG_MAIN_AON_CPU_INTR",    INT_ENABLE_MASK}, /* 25 */
    {"UPG_SC_CPU_INTR",          INT_ENABLE_MASK}, /* 26 */
    {"UPG_SPI_CPU_INTR",       	 INT_ENABLE_MASK}, /* 27 */
    {"UPG_TMR_CPU_INTR",         INT_ENABLE_MASK}, /* 28 */
    {"UPG_UART0_CPU_INTR",       0}, /* 29 */
    {"UPG_UART1_CPU_INTR",       0}, /* 30 */
    {"UPG_UART2_CPU_INTR",       0}, /* 31 */
    {"USB0_BRIDGE_CPU_INTR",     0 | INT_REG_WRAP}, /* 0 */
    {"USB0_EHCI_0_CPU_INTR",     0}, /* 1 */
    {"USB0_OHCI_0_CPU_INTR",     0}, /* 2 */
    {"SPARE_09_CPU_INTR",        0}, /* 3 */    
    {"SPARE_10_CPU_INTR",        0}, /* 4 */    
    {"SPARE_11_CPU_INTR",        0}, /* 5 */    
    {"SPARE_12_CPU_INTR",        0}, /* 6 */    
    {"SPARE_13_CPU_INTR",        0}, /* 7 */    
    {"SPARE_14_CPU_INTR",        0}, /* 8 */    
    {"SPARE_15_CPU_INTR",        0}, /* 9 */    
    {"VEC_CPU_INTR",      	     INT_ENABLE_MASK}, /* 10 */
    {"XPT_FE_CPU_INTR",          INT_ENABLE_MASK}, /* 11 */
    {"XPT_MSG_CPU_INTR",   		 INT_ENABLE_MASK}, /* 12 */
    {"XPT_MSG_STAT_CPU_INTR",    INT_ENABLE_MASK}, /* 13 */
    {"XPT_OVFL_CPU_INTR",        INT_ENABLE_MASK}, /* 14 */
    {"XPT_PCR_CPU_INTR",         INT_ENABLE_MASK}, /* 15 */
    {"XPT_RAV_CPU_INTR",         INT_ENABLE_MASK}, /* 16 */
    {"XPT_STATUS_CPU_INTR",      INT_ENABLE_MASK}, /* 17 */
    {"SDIO0_0_CPU_INTR",         0}, /* 18 */
    {"SPARE_16_CPU_INTR",        0}, /* 19 */    
    {"AVD0_0_CPU_INTR",      	 INT_ENABLE_MASK}, /* 20 */
    {"SPARE_17_CPU_INTR",        0}, /* 21 */    
    {"SPARE_18_CPU_INTR",        0}, /* 22 */    
    {"RFM_CPU_INTR",             0}, /* 23*/    
    {"THD_A_CPU_INTR",           0}, /* 24 */    
    {"THD_B_CPU_INTR",           0}, /* 25 */    
    {"UFE_CPU_INTR",             0}, /* 26 */    
    {"DS_0_CPU_INTR",            0}, /* 27 */    
    {"SPARE_19_CPU_INTR",        0}, /* 28 */    
    {"SPARE_20_CPU_INTR",        0}, /* 29 */    
    {"MCIF_CPU_INTR",            0}, /* 30 */    
    {"UPG_AUX_CPU_INTR",      	 INT_ENABLE_MASK}, /* 31 */
};


s_ChipConfig  g_sChipConfig ={ "7358",interruptName7358, 0xb0411400, sizeof(interruptName7358)/sizeof(s_InteruptTable),NUM_INTC_WORDS};
