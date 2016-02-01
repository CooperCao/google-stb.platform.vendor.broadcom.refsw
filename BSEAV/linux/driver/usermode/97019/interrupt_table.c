/***************************************************************************
 *     Copyright (c) 2002-2012, Broadcom Corporation
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
 * Module Description:
 * This file contains the platform specific interrupt
 *   tables and shread functions
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
#elif ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) ) && ( LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37) )
#include <linux/autoconf.h>
#endif
#include <linux/module.h>
#include "bcmdriver_common.h"

#define NUM_INTC_WORDS   3   /* Number of interrupt controller words */

s_InteruptTable interruptName7125[] = {
    {NULL, 0},                   /* Linux IRQ #'s are offset by 1 */
    {"HIF_CPU_INTR",                0},/* 0 (1) */
    {"XPT_STATUS_CPU_INTR",         INT_ENABLE_MASK},/* 1 (2)*/
    {"XPT_OVFL_CPU_INTR",           INT_ENABLE_MASK},/* 2 (3)*/
    {"XPT_MSG_CPU_INTR",            INT_ENABLE_MASK},/* 3 (4)*/
    {"XPT_ICAM_CPU_INTR",           INT_ENABLE_MASK},/* 4 (5)*/
    {"BSP_CPU_INTR",                INT_ENABLE_MASK},/* 5 (6)*/
    {"AIO_CPU_INTR",                INT_ENABLE_MASK},/* 6 (7)*/
    {"GFX_CPU_INTR",                INT_ENABLE_MASK},/* 7 (8)*/
    {"HDMI_CPU_INTR",               INT_ENABLE_MASK},/* 8 (9)*/
    {"RPTD_CPU_INTR",               INT_ENABLE_MASK},/* 9 (10)*/
    {"VEC_CPU_INTR",                INT_ENABLE_MASK},/* 10 (11)*/
    {"BVNB_CPU_INTR",               INT_ENABLE_MASK},/* 11 (12)*/
    {"BVNF_CPU_INTR_0",             INT_ENABLE_MASK},/* 12 (13)*/
    {"BVNF_CPU_INTR_1",             INT_ENABLE_MASK},/* 13 (14)*/
    {"MEMC_0_CPU_INTR",             INT_ENABLE_MASK},/* 14 (15)*/
    {"HIF_SPI_CPU_INTR",            0},/* 15 (16)*/
    {NULL,                          0},/* 16 (17)*/
    {"UPG_TMR_CPU_INTR",            INT_ENABLE_MASK}, /* 17 (18)*/
    {"UPG_CPU_INTR",                INT_ENABLE_MASK}, /* 18 (19)*/
    {"UPG_BSC_CPU_INTR",            INT_ENABLE_MASK}, /* 19 (20)*/
    {"UPG_SPI_CPU_INTR",            INT_ENABLE_MASK}, /* 20 (21)*/
    {"UPG_UART0_CPU_INTR",          0}, /* 21 (22)*/
    {"UPG_SC_CPU_INTR",             INT_ENABLE_MASK}, /* 22 (23)*/
    {"SUN_CPU_INTR",                INT_ENABLE_MASK}, /* 23 (24)*/
    {NULL,                          0},/* 24 (25)*/
    {NULL,                          0},/* 25 (26)*/
    {"USB_CPU_INTR",                0},/* 26 (27)*/
    {"MC_CPU_INTR",                 INT_ENABLE_MASK}, /* 27 (28)*/
    {"BVNF_CPU_INTR_5",             INT_ENABLE_MASK}, /* 28 (29)*/
    {"SATA_PCIB_CPU_INTR",          0}, /* 29 (30)*/
    {"AVD0_CPU_INTR",               INT_ENABLE_MASK}, /* 30 (31)*/
    {"XPT_RAV_CPU_INTR",            INT_ENABLE_MASK}, /* 31 (32)*/
    {"PCI_INTA_0_CPU_INTR",         0 | INT_REG_WRAP}, /* 0 (33)*/
    {"PCI_INTA_1_CPU_INTR",         0}, /* 1 (34)*/
    {"PCI_INTA_2_CPU_INTR",         0}, /* 2 (35)*/
    {NULL,                          0}, /* 3 (36)*/
    {"EXT_IRQ_0_CPU_INTR",          INT_ENABLE_MASK}, /* 4 (37)*/
    {"EXT_IRQ_1_CPU_INTR",          INT_ENABLE_MASK}, /* 5 (38)*/
    {"EXT_IRQ_2_CPU_INTR",          INT_ENABLE_MASK}, /* 6 (39)*/
    {"EXT_IRQ_3_CPU_INTR",          INT_ENABLE_MASK}, /* 7 (40)*/
    {"EXT_IRQ_4_CPU_INTR",          INT_ENABLE_MASK}, /* 8 (41)*/
    {"PCI_SATA_CPU_INTR",           0}, /* 9 (42)*/
    {"EXT_IRQ_5_CPU_INTR",          INT_ENABLE_MASK}, /* 10 (43)*/
    {"EXT_IRQ_6_CPU_INTR",          0}, /* 11 (44)*/
    {"EXT_IRQ_7_CPU_INTR",          0}, /* 12 (45)*/
    {"EXT_IRQ_8_CPU_INTR",          0}, /* 13 (46)*/
    {"EXT_IRQ_9_CPU_INTR",          0}, /* 14 (47)*/
    {"EXT_IRQ_10_CPU_INTR",         0}, /* 15 (48)*/
    {"EXT_IRQ_11_CPU_INTR",         0}, /* 16 (49)*/
    {"EXT_IRQ_12_CPU_INTR",         INT_ENABLE_MASK}, /* 17 (50)*/
    {"EXT_IRQ_13_CPU_INTR",         INT_ENABLE_MASK}, /* 18 (51)*/
    {"EXT_IRQ_14_CPU_INTR",         0}, /* 19 (52)*/
    {"IPI0_CPU_INTR",               INT_ENABLE_MASK}, /* 20 (53)*/
    {"IPI1_CPU_INTR",               INT_ENABLE_MASK}, /* 21 (54)*/
    {NULL,                          0}, /* 22 (55)*/
    {NULL,                          0}, /* 23 (56)*/
    {NULL,                          0}, /* 24 (57)*/
    {"XPT_PCR_CPU_INTR",            INT_ENABLE_MASK}, /* 25 (58)*/
    {"XPT_FE_CPU_INTR",             INT_ENABLE_MASK}, /* 26 (59)*/
    {"XPT_MSG_STAT_CPU_INTR",       INT_ENABLE_MASK}, /* 27 (60)*/
    {"USB_EHCI_0_CPU_INTR",         0}, /* 28 (61)*/
    {"USB_OHCI_0_CPU_INTR",         0}, /* 29 (62)*/
    {NULL,                          0}, /* 30 (63)*/
    {"BNM_CPU_INTR",                0}, /* 31 (64)*/
    {"UPG_UART1_CPU_INTR",          0}, /* 0 (65)*/
    {"UPG_UART2_CPU_INTR",          0}, /* 1 (66)*/
    {"NMI_PIN_CPU_INTR",            INT_ENABLE_MASK}, /* 2 (67)*/
    {"MOCA_INTR_CPU_INTR",          0}, /* 3 (68)*/
    {"MOCA_GENET_0_A_INTR_CPU_INTR",0}, /* 4 (69)*/
    {"SUNDRY_PM_INTR_CPU_INTR",     INT_ENABLE_MASK}, /* 5 (70)*/
    {NULL,                          0}, /* 6 (71)*/
    {NULL,                          0}, /* 7 (72)*/
    {NULL,                          0}, /* 8 (73)*/
    {NULL,                          0}, /* 9 (74)*/
    {NULL,                          0}, /* 10 (75)*/
    {NULL,                          0}, /* 11 (76)*/
    {NULL,                          0}, /* 12 (77)*/
    {NULL,                          0}, /* 13 (78)*/
    {NULL,                          0}, /* 14 (79)*/
    {NULL,                          0}, /* 15 (80)*/
    {"MOCA_GENET_0_B_INTR_CPU_INTR",0}, /* 16 (81)*/
    {NULL,                          0}, /* 17 (82)*/
    {NULL,                          0}, /* 18 (83)*/
    {NULL,                          0}, /* 19 (84)*/
    {NULL,                          0}, /* 20 (85)*/
    {NULL,                          0}, /* 21 (86)*/
    {NULL,                          0}, /* 22 (87)*/
    {NULL,                          0}, /* 23 (88)*/
    {NULL,                          0}, /* 24 (89)*/
    {NULL,                          0}, /* 25 (90)*/
    {NULL,                          0}, /* 26 (91)*/
    {NULL,                          0}, /* 27 (92)*/
    {NULL,                          0}, /* 28 (93)*/
    {NULL,                          0}, /* 29 (94)*/
    {NULL,                          0}, /* 30 (95)*/
    {NULL,                          0}  /* 31 (96)*/
};


s_ChipConfig  g_sChipConfig ={ "7125",interruptName7125, 0xb0441400, sizeof(interruptName7125)/sizeof(s_InteruptTable),NUM_INTC_WORDS};
