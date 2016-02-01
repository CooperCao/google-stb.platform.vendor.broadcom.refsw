/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 *
 * Module Description:
 * This file contains the platform specific interrupt tables 
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
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


s_InteruptTable interruptName7408[] = {
    {NULL, 0},                   /* Linux IRQ #'s are offset by 1 */
    {"HIF_CPU_INTR",             0},/* 0 */
    {"XPT_STATUS_CPU_INTR",      INT_ENABLE_MASK},/* 1 */
    {"XPT_MDMA_CPU_INTR",        INT_ENABLE_MASK},/* 2 */
    {"XPT_JTAG_CPU_INTR",        0},/* 3 */
    {NULL,                       0},/* 4 */
    {NULL,                       0},/* 5 */
    {"AIO_CPU_INTR",             INT_ENABLE_MASK},/* 6 */
    {"GFX_CPU_INTR",             INT_ENABLE_MASK},/* 7 */
    {"HDMI_CPU_INTR",            INT_ENABLE_MASK},/* 8 */
    {NULL,                       0},/* 9 */
    {"VEC_CPU_INTR",             INT_ENABLE_MASK},/* 10 */
    {"BVNB_CPU_INTR",            INT_ENABLE_MASK},/* 11 */
    {"BVNF_0_CPU_INTR",          INT_ENABLE_MASK},/* 12 */
    {"BVNF_1_CPU_INTR",          INT_ENABLE_MASK},/* 13 */
    {"MEMC_0_CPU_INTR",          INT_ENABLE_MASK},/* 14 */
    {NULL,                       0},/* 15 */
    {NULL,                       0},/* 16 */
    {"UPG_TMR_CPU_INTR",         INT_ENABLE_MASK},/* 17 */
    {"UPG_CPU_INTR",             INT_ENABLE_MASK}, /* 18 */
    {"UPG_BSC_CPU_INTR",         INT_ENABLE_MASK}, /* 19 */
    {NULL,                       0}, /* 20 */
    {"UPG_UART0_CPU_INTR",       0}, /* 21 */
    {NULL,                       0}, /* 22 */
    {"SUN_CPU_INTR",             INT_ENABLE_MASK}, /* 23 */
    {"UHF1_CPU_INTR",            INT_ENABLE_MASK}, /* 24 */
    {NULL,                       0}, /* 25 */
    {"USB_CPU_INTR",             0}, /* 26 */
    {"MC_CPU_INTR",              INT_ENABLE_MASK}, /* 27 */
    {"BVNF_5_CPU_INTR",          INT_ENABLE_MASK}, /* 28 */
    {NULL,                       0}, /* 29 */
    {"AVD0_CPU_INTR",            INT_ENABLE_MASK}, /* 30 */
    {"XPT_RAV_CPU_INTR",         INT_ENABLE_MASK}, /* 31 */
    {NULL,                       0  | INT_REG_WRAP}, /* 0 */
    {NULL,                       0}, /* 1 */
    {NULL,                       0}, /* 2 */
    {NULL,                       0}, /* 3 */
    {"EXT_IRQ_0_CPU_INTR",       INT_ENABLE_MASK}, /* 4 */
    {"EXT_IRQ_1_CPU_INTR",       0}, /* 5 */
    {"EXT_IRQ_2_CPU_INTR",       INT_ENABLE_MASK}, /* 6, SLOT 2 */
    {"EXT_IRQ_3_CPU_INTR",       INT_ENABLE_MASK}, /* 7 */
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
    {"IPI0_CPU_INTR",            INT_ENABLE_MASK}, /* 20 */
    {"IPI1_CPU_INTR",            INT_ENABLE_MASK}, /* 21 */
    {NULL,                       0}, /* 22 */
    {NULL,                       0}, /* 23 */
    {NULL,                       0}, /* 24 */
    {"XPT_PCR_CPU_INTR",         INT_ENABLE_MASK}, /* 25 */
    {"XPT_FE_CPU_INTR",          INT_ENABLE_MASK}, /* 26 */
    {NULL,                       0}, /* 27 */
    {"USB_EHCI_0_CPU_INTR",      0}, /* 28 */
    {"USB_OHCI_0_CPU_INTR",      0}, /* 29 */
    {NULL,                       0}, /* 30 */
    {NULL,                       0}, /* 31 */
    {"UPG_UART1_CPU_INTR",       0 | INT_REG_WRAP}, /* 0 */
    {"UPG_UART2_CPU_INTR",       0}, /* 1 */
    {"NMI_INTR_CPU_INTR",        INT_ENABLE_MASK}, /* 2 */
    {"MOCA_INTR_CPU_INTR",       0}, /* 3 */
    {"MOCA_GENET_0_A_CPU_INTR",  0}, /* 4 */
    {"SUNDRY_PM_INTR_CPU_INTR",  INT_ENABLE_MASK}, /* 5 */
    {NULL,                       0}, /* 6 */
    {"BVNF_6_CPU_INTR",          INT_ENABLE_MASK}, /* 7 */
    {NULL,                       0}, /* 8 */
    {NULL,                       0}, /* 9 */
    {NULL,                       0}, /* 10 */
    {NULL,                       0}, /* 11 */
    {NULL,                       0}, /* 12 */
    {NULL,                       0}, /* 13 */
    {"HIF_SPI_CPU_INTR",         0}, /* 14 */
    {NULL,                       0}, /* 15 */
    {"MOCA_GENET_0_B_CPU_INTR",  0}, /* 16 */
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


s_ChipConfig  g_sChipConfig ={ "7408",interruptName7408, 0xb0441400, sizeof(interruptName7408)/sizeof(s_InteruptTable),NUM_INTC_WORDS};
