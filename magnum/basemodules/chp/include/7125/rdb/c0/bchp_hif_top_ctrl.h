/***************************************************************************
 *     Copyright (c) 1999-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on         Fri Jan 22 20:12:26 2010
 *                 MD5 Checksum         a2d1f2163f65e87d228a0fb491cb442d
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008008
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_HIF_TOP_CTRL_H__
#define BCHP_HIF_TOP_CTRL_H__

/***************************************************************************
 *HIF_TOP_CTRL - HIF Top Control Registers
 ***************************************************************************/
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL          0x00442400 /* External IRQ Active Level Control Register */
#define BCHP_HIF_TOP_CTRL_TM_CTRL                0x00442404 /* HIF MBIST_TM_CTRL Register */
#define BCHP_HIF_TOP_CTRL_SCRATCH                0x00442408 /* HIF Scratch Register */
#define BCHP_HIF_TOP_CTRL_PM_CTRL                0x0044240c /* HIF Power Management Control Register */
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT        0x00442410 /* HIF Strap Intercept Register */
#define BCHP_HIF_TOP_CTRL_SPI_CLK_SEL            0x00442414 /* HIF SPI Interface Clock Select Register */
#define BCHP_HIF_TOP_CTRL_FLASH_TYPE             0x00442418 /* HIF Decoded Flash Type */
#define BCHP_HIF_TOP_CTRL_SPI_DBG_SEL            0x0044241c /* SPI test port select register */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL     0x00442420 /* HIF CRC control register for MISB and LLMB */
#define BCHP_HIF_TOP_CTRL_CRC_LLMB_DATA0         0x00442424 /* HIF CRC LLMB result register0 */
#define BCHP_HIF_TOP_CTRL_CRC_LLMB_DATA1         0x00442428 /* HIF CRC LLMB result register1 */
#define BCHP_HIF_TOP_CTRL_CRC_MISBSTORE_DATA0    0x0044242c /* HIF CRC MISB store data bus result register0 */
#define BCHP_HIF_TOP_CTRL_CRC_MISBSTORE_DATA1    0x00442430 /* HIF CRC MISB store data bus result register1 */
#define BCHP_HIF_TOP_CTRL_CRC_MISBLOAD_DATA0     0x00442434 /* HIF CRC MISB store data bus result register0 */
#define BCHP_HIF_TOP_CTRL_CRC_MISBLOAD_DATA1     0x00442438 /* HIF CRC MISB store data bus result register1 */
#define BCHP_HIF_TOP_CTRL_CRC_LLMB_ADDR          0x0044243c /* HIF CRC LLMB address bus result register */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_ADDR          0x00442440 /* HIF CRC MISB address bus result register */
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL          0x00442444 /* HIF PCI ConfigurationManager  MWIN Control Register */

/***************************************************************************
 *EXT_IRQ_LEVEL - External IRQ Active Level Control Register
 ***************************************************************************/
/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: reserved0 [31:15] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_reserved0_MASK             0xffff8000
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_reserved0_SHIFT            15

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_14_level [14:14] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_14_level_MASK      0x00004000
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_14_level_SHIFT     14
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_14_level_LOW       0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_14_level_HIGH      1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_13_level [13:13] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_13_level_MASK      0x00002000
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_13_level_SHIFT     13
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_13_level_LOW       0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_13_level_HIGH      1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_12_level [12:12] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_12_level_MASK      0x00001000
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_12_level_SHIFT     12
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_12_level_LOW       0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_12_level_HIGH      1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_11_level [11:11] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_11_level_MASK      0x00000800
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_11_level_SHIFT     11
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_11_level_LOW       0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_11_level_HIGH      1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_10_level [10:10] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_10_level_MASK      0x00000400
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_10_level_SHIFT     10
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_10_level_LOW       0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_10_level_HIGH      1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_9_level [09:09] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_9_level_MASK       0x00000200
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_9_level_SHIFT      9
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_9_level_LOW        0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_9_level_HIGH       1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_8_level [08:08] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_8_level_MASK       0x00000100
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_8_level_SHIFT      8
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_8_level_LOW        0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_8_level_HIGH       1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_7_level [07:07] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_7_level_MASK       0x00000080
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_7_level_SHIFT      7
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_7_level_LOW        0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_7_level_HIGH       1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_6_level [06:06] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_6_level_MASK       0x00000040
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_6_level_SHIFT      6
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_6_level_LOW        0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_6_level_HIGH       1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_5_level [05:05] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_5_level_MASK       0x00000020
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_5_level_SHIFT      5
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_5_level_LOW        0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_5_level_HIGH       1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_4_level [04:04] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_4_level_MASK       0x00000010
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_4_level_SHIFT      4
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_4_level_LOW        0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_4_level_HIGH       1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_3_level [03:03] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_3_level_MASK       0x00000008
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_3_level_SHIFT      3
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_3_level_LOW        0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_3_level_HIGH       1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_2_level [02:02] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_2_level_MASK       0x00000004
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_2_level_SHIFT      2
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_2_level_LOW        0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_2_level_HIGH       1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_1_level [01:01] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_1_level_MASK       0x00000002
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_1_level_SHIFT      1
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_1_level_LOW        0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_1_level_HIGH       1

/* HIF_TOP_CTRL :: EXT_IRQ_LEVEL :: ext_irq_0_level [00:00] */
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_0_level_MASK       0x00000001
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_0_level_SHIFT      0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_0_level_LOW        0
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_0_level_HIGH       1

/***************************************************************************
 *TM_CTRL - HIF MBIST_TM_CTRL Register
 ***************************************************************************/
/* HIF_TOP_CTRL :: TM_CTRL :: reserved0 [31:16] */
#define BCHP_HIF_TOP_CTRL_TM_CTRL_reserved0_MASK                   0xffff0000
#define BCHP_HIF_TOP_CTRL_TM_CTRL_reserved0_SHIFT                  16

/* HIF_TOP_CTRL :: TM_CTRL :: MPI_ECC_FIFO [15:14] */
#define BCHP_HIF_TOP_CTRL_TM_CTRL_MPI_ECC_FIFO_MASK                0x0000c000
#define BCHP_HIF_TOP_CTRL_TM_CTRL_MPI_ECC_FIFO_SHIFT               14
#define BCHP_HIF_TOP_CTRL_TM_CTRL_MPI_ECC_FIFO_DISABLE             0
#define BCHP_HIF_TOP_CTRL_TM_CTRL_MPI_ECC_FIFO_ENABLE              1

/* HIF_TOP_CTRL :: TM_CTRL :: MPI_RGBUF [13:10] */
#define BCHP_HIF_TOP_CTRL_TM_CTRL_MPI_RGBUF_MASK                   0x00003c00
#define BCHP_HIF_TOP_CTRL_TM_CTRL_MPI_RGBUF_SHIFT                  10

/* HIF_TOP_CTRL :: TM_CTRL :: MPI_NAND_FIFO [09:08] */
#define BCHP_HIF_TOP_CTRL_TM_CTRL_MPI_NAND_FIFO_MASK               0x00000300
#define BCHP_HIF_TOP_CTRL_TM_CTRL_MPI_NAND_FIFO_SHIFT              8
#define BCHP_HIF_TOP_CTRL_TM_CTRL_MPI_NAND_FIFO_DISABLE            0
#define BCHP_HIF_TOP_CTRL_TM_CTRL_MPI_NAND_FIFO_ENABLE             1

/* HIF_TOP_CTRL :: TM_CTRL :: WRITE_FIFO_TM0 [07:06] */
#define BCHP_HIF_TOP_CTRL_TM_CTRL_WRITE_FIFO_TM0_MASK              0x000000c0
#define BCHP_HIF_TOP_CTRL_TM_CTRL_WRITE_FIFO_TM0_SHIFT             6
#define BCHP_HIF_TOP_CTRL_TM_CTRL_WRITE_FIFO_TM0_DISABLE           0
#define BCHP_HIF_TOP_CTRL_TM_CTRL_WRITE_FIFO_TM0_ENABLE            1

/* HIF_TOP_CTRL :: TM_CTRL :: WRITE_FIFO_TM1 [05:04] */
#define BCHP_HIF_TOP_CTRL_TM_CTRL_WRITE_FIFO_TM1_MASK              0x00000030
#define BCHP_HIF_TOP_CTRL_TM_CTRL_WRITE_FIFO_TM1_SHIFT             4
#define BCHP_HIF_TOP_CTRL_TM_CTRL_WRITE_FIFO_TM1_DISABLE           0
#define BCHP_HIF_TOP_CTRL_TM_CTRL_WRITE_FIFO_TM1_ENABLE            1

/* HIF_TOP_CTRL :: TM_CTRL :: READ_FIFO_TM0 [03:02] */
#define BCHP_HIF_TOP_CTRL_TM_CTRL_READ_FIFO_TM0_MASK               0x0000000c
#define BCHP_HIF_TOP_CTRL_TM_CTRL_READ_FIFO_TM0_SHIFT              2
#define BCHP_HIF_TOP_CTRL_TM_CTRL_READ_FIFO_TM0_DISABLE            0
#define BCHP_HIF_TOP_CTRL_TM_CTRL_READ_FIFO_TM0_ENABLE             1

/* HIF_TOP_CTRL :: TM_CTRL :: READ_FIFO_TM1 [01:00] */
#define BCHP_HIF_TOP_CTRL_TM_CTRL_READ_FIFO_TM1_MASK               0x00000003
#define BCHP_HIF_TOP_CTRL_TM_CTRL_READ_FIFO_TM1_SHIFT              0
#define BCHP_HIF_TOP_CTRL_TM_CTRL_READ_FIFO_TM1_DISABLE            0
#define BCHP_HIF_TOP_CTRL_TM_CTRL_READ_FIFO_TM1_ENABLE             1

/***************************************************************************
 *SCRATCH - HIF Scratch Register
 ***************************************************************************/
/* HIF_TOP_CTRL :: SCRATCH :: SCRATCH_BIT [31:00] */
#define BCHP_HIF_TOP_CTRL_SCRATCH_SCRATCH_BIT_MASK                 0xffffffff
#define BCHP_HIF_TOP_CTRL_SCRATCH_SCRATCH_BIT_SHIFT                0

/***************************************************************************
 *PM_CTRL - HIF Power Management Control Register
 ***************************************************************************/
/* HIF_TOP_CTRL :: PM_CTRL :: reserved0 [31:14] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_reserved0_MASK                   0xffffc000
#define BCHP_HIF_TOP_CTRL_PM_CTRL_reserved0_SHIFT                  14

/* HIF_TOP_CTRL :: PM_CTRL :: EBI_PM_IN_DRIVE_INACTIVE [13:13] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_EBI_PM_IN_DRIVE_INACTIVE_MASK    0x00002000
#define BCHP_HIF_TOP_CTRL_PM_CTRL_EBI_PM_IN_DRIVE_INACTIVE_SHIFT   13

/* HIF_TOP_CTRL :: PM_CTRL :: PCI_PM_IN_DRIVE_INACTIVE [12:12] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PCI_PM_IN_DRIVE_INACTIVE_MASK    0x00001000
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PCI_PM_IN_DRIVE_INACTIVE_SHIFT   12

/* HIF_TOP_CTRL :: PM_CTRL :: MPI_PM_IN_DRIVE_INACTIVE [11:11] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_MPI_PM_IN_DRIVE_INACTIVE_MASK    0x00000800
#define BCHP_HIF_TOP_CTRL_PM_CTRL_MPI_PM_IN_DRIVE_INACTIVE_SHIFT   11

/* HIF_TOP_CTRL :: PM_CTRL :: PM_OUT_TRISTATE_CS6 [10:10] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS6_MASK         0x00000400
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS6_SHIFT        10

/* HIF_TOP_CTRL :: PM_CTRL :: PM_OUT_TRISTATE_CS5 [09:09] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS5_MASK         0x00000200
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS5_SHIFT        9

/* HIF_TOP_CTRL :: PM_CTRL :: PM_OUT_TRISTATE_CS4 [08:08] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS4_MASK         0x00000100
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS4_SHIFT        8

/* HIF_TOP_CTRL :: PM_CTRL :: PM_OUT_TRISTATE_CS3 [07:07] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS3_MASK         0x00000080
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS3_SHIFT        7

/* HIF_TOP_CTRL :: PM_CTRL :: PM_OUT_TRISTATE_CS2 [06:06] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS2_MASK         0x00000040
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS2_SHIFT        6

/* HIF_TOP_CTRL :: PM_CTRL :: PM_OUT_TRISTATE_CS1 [05:05] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS1_MASK         0x00000020
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS1_SHIFT        5

/* HIF_TOP_CTRL :: PM_CTRL :: PM_OUT_TRISTATE_CS0 [04:04] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS0_MASK         0x00000010
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_CS0_SHIFT        4

/* HIF_TOP_CTRL :: PM_CTRL :: PM_OUT_TRISTATE_PCIGNT [03:03] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_PCIGNT_MASK      0x00000008
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PM_OUT_TRISTATE_PCIGNT_SHIFT     3

/* HIF_TOP_CTRL :: PM_CTRL :: EBI_PM_OUT_DRIVE_LOW [02:02] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_EBI_PM_OUT_DRIVE_LOW_MASK        0x00000004
#define BCHP_HIF_TOP_CTRL_PM_CTRL_EBI_PM_OUT_DRIVE_LOW_SHIFT       2

/* HIF_TOP_CTRL :: PM_CTRL :: PCI_PM_OUT_DRIVE_LOW [01:01] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PCI_PM_OUT_DRIVE_LOW_MASK        0x00000002
#define BCHP_HIF_TOP_CTRL_PM_CTRL_PCI_PM_OUT_DRIVE_LOW_SHIFT       1

/* HIF_TOP_CTRL :: PM_CTRL :: MPI_PM_OUT_DRIVE_LOW [00:00] */
#define BCHP_HIF_TOP_CTRL_PM_CTRL_MPI_PM_OUT_DRIVE_LOW_MASK        0x00000001
#define BCHP_HIF_TOP_CTRL_PM_CTRL_MPI_PM_OUT_DRIVE_LOW_SHIFT       0

/***************************************************************************
 *STRAP_INTERCEPT - HIF Strap Intercept Register
 ***************************************************************************/
/* HIF_TOP_CTRL :: STRAP_INTERCEPT :: reserved0 [31:05] */
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_reserved0_MASK           0xffffffe0
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_reserved0_SHIFT          5

/* HIF_TOP_CTRL :: STRAP_INTERCEPT :: STRAP_EBI_BOOT_MEMORY [04:04] */
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_STRAP_EBI_BOOT_MEMORY_MASK 0x00000010
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_STRAP_EBI_BOOT_MEMORY_SHIFT 4

/* HIF_TOP_CTRL :: STRAP_INTERCEPT :: STRAP_EBI_BOOT_MEMORY_INTERCEPT_EN [03:03] */
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_STRAP_EBI_BOOT_MEMORY_INTERCEPT_EN_MASK 0x00000008
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_STRAP_EBI_BOOT_MEMORY_INTERCEPT_EN_SHIFT 3

/* HIF_TOP_CTRL :: STRAP_INTERCEPT :: STRAP_NAND_FLASH [02:02] */
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_STRAP_NAND_FLASH_MASK    0x00000004
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_STRAP_NAND_FLASH_SHIFT   2

/* HIF_TOP_CTRL :: STRAP_INTERCEPT :: STRAP_NAND_FLASH_INTERCEPT_EN [01:01] */
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_STRAP_NAND_FLASH_INTERCEPT_EN_MASK 0x00000002
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_STRAP_NAND_FLASH_INTERCEPT_EN_SHIFT 1

/* HIF_TOP_CTRL :: STRAP_INTERCEPT :: STRAP_PCI_EXT_ARB [00:00] */
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_STRAP_PCI_EXT_ARB_MASK   0x00000001
#define BCHP_HIF_TOP_CTRL_STRAP_INTERCEPT_STRAP_PCI_EXT_ARB_SHIFT  0

/***************************************************************************
 *SPI_CLK_SEL - HIF SPI Interface Clock Select Register
 ***************************************************************************/
/* HIF_TOP_CTRL :: SPI_CLK_SEL :: reserved0 [31:02] */
#define BCHP_HIF_TOP_CTRL_SPI_CLK_SEL_reserved0_MASK               0xfffffffc
#define BCHP_HIF_TOP_CTRL_SPI_CLK_SEL_reserved0_SHIFT              2

/* HIF_TOP_CTRL :: SPI_CLK_SEL :: BSPI_CLK_FREQ_SEL [01:00] */
#define BCHP_HIF_TOP_CTRL_SPI_CLK_SEL_BSPI_CLK_FREQ_SEL_MASK       0x00000003
#define BCHP_HIF_TOP_CTRL_SPI_CLK_SEL_BSPI_CLK_FREQ_SEL_SHIFT      0

/***************************************************************************
 *FLASH_TYPE - HIF Decoded Flash Type
 ***************************************************************************/
/* HIF_TOP_CTRL :: FLASH_TYPE :: reserved0 [31:04] */
#define BCHP_HIF_TOP_CTRL_FLASH_TYPE_reserved0_MASK                0xfffffff0
#define BCHP_HIF_TOP_CTRL_FLASH_TYPE_reserved0_SHIFT               4

/* HIF_TOP_CTRL :: FLASH_TYPE :: InvalidStrap [03:03] */
#define BCHP_HIF_TOP_CTRL_FLASH_TYPE_InvalidStrap_MASK             0x00000008
#define BCHP_HIF_TOP_CTRL_FLASH_TYPE_InvalidStrap_SHIFT            3

/* HIF_TOP_CTRL :: FLASH_TYPE :: IS_PCI_CLIENT_MODE [02:02] */
#define BCHP_HIF_TOP_CTRL_FLASH_TYPE_IS_PCI_CLIENT_MODE_MASK       0x00000004
#define BCHP_HIF_TOP_CTRL_FLASH_TYPE_IS_PCI_CLIENT_MODE_SHIFT      2

/* HIF_TOP_CTRL :: FLASH_TYPE :: FLASH_TYPE [01:00] */
#define BCHP_HIF_TOP_CTRL_FLASH_TYPE_FLASH_TYPE_MASK               0x00000003
#define BCHP_HIF_TOP_CTRL_FLASH_TYPE_FLASH_TYPE_SHIFT              0

/***************************************************************************
 *SPI_DBG_SEL - SPI test port select register
 ***************************************************************************/
/* HIF_TOP_CTRL :: SPI_DBG_SEL :: reserved0 [31:03] */
#define BCHP_HIF_TOP_CTRL_SPI_DBG_SEL_reserved0_MASK               0xfffffff8
#define BCHP_HIF_TOP_CTRL_SPI_DBG_SEL_reserved0_SHIFT              3

/* HIF_TOP_CTRL :: SPI_DBG_SEL :: DISABLE_MSPI_FLUSH [02:02] */
#define BCHP_HIF_TOP_CTRL_SPI_DBG_SEL_DISABLE_MSPI_FLUSH_MASK      0x00000004
#define BCHP_HIF_TOP_CTRL_SPI_DBG_SEL_DISABLE_MSPI_FLUSH_SHIFT     2

/* HIF_TOP_CTRL :: SPI_DBG_SEL :: SPI_RBUS_TIMER_EN [01:01] */
#define BCHP_HIF_TOP_CTRL_SPI_DBG_SEL_SPI_RBUS_TIMER_EN_MASK       0x00000002
#define BCHP_HIF_TOP_CTRL_SPI_DBG_SEL_SPI_RBUS_TIMER_EN_SHIFT      1

/* HIF_TOP_CTRL :: SPI_DBG_SEL :: SPI_TP_SEL [00:00] */
#define BCHP_HIF_TOP_CTRL_SPI_DBG_SEL_SPI_TP_SEL_MASK              0x00000001
#define BCHP_HIF_TOP_CTRL_SPI_DBG_SEL_SPI_TP_SEL_SHIFT             0

/***************************************************************************
 *CRC_MISB_LLMB_CTRL - HIF CRC control register for MISB and LLMB
 ***************************************************************************/
/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: reserved0 [31:12] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_reserved0_MASK        0xfffff000
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_reserved0_SHIFT       12

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: LLMB_CRC_MODE [11:10] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_CRC_MODE_MASK    0x00000c00
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_CRC_MODE_SHIFT   10

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: MISB_ADDR_CRC_RESET [09:09] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_ADDR_CRC_RESET_MASK 0x00000200
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_ADDR_CRC_RESET_SHIFT 9
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_ADDR_CRC_RESET_DISABLE 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_ADDR_CRC_RESET_ENABLE 1

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: MISB_ADDR_CRC_ENABLE [08:08] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_ADDR_CRC_ENABLE_MASK 0x00000100
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_ADDR_CRC_ENABLE_SHIFT 8
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_ADDR_CRC_ENABLE_DISABLE 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_ADDR_CRC_ENABLE_ENABLE 1

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: LLMB_ADDR_CRC_RESET [07:07] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_ADDR_CRC_RESET_MASK 0x00000080
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_ADDR_CRC_RESET_SHIFT 7
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_ADDR_CRC_RESET_DISABLE 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_ADDR_CRC_RESET_ENABLE 1

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: LLMB_ADDR_CRC_ENABLE [06:06] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_ADDR_CRC_ENABLE_MASK 0x00000040
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_ADDR_CRC_ENABLE_SHIFT 6
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_ADDR_CRC_ENABLE_DISABLE 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_ADDR_CRC_ENABLE_ENABLE 1

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: MISB_DATALOAD_CRC_RESET [05:05] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATALOAD_CRC_RESET_MASK 0x00000020
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATALOAD_CRC_RESET_SHIFT 5
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATALOAD_CRC_RESET_DISABLE 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATALOAD_CRC_RESET_ENABLE 1

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: MISB_DATALOAD_CRC_ENABLE [04:04] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATALOAD_CRC_ENABLE_MASK 0x00000010
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATALOAD_CRC_ENABLE_SHIFT 4
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATALOAD_CRC_ENABLE_DISABLE 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATALOAD_CRC_ENABLE_ENABLE 1

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: MISB_DATASTORE_CRC_RESET [03:03] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATASTORE_CRC_RESET_MASK 0x00000008
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATASTORE_CRC_RESET_SHIFT 3
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATASTORE_CRC_RESET_DISABLE 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATASTORE_CRC_RESET_ENABLE 1

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: MISB_DATASTORE_CRC_ENABLE [02:02] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATASTORE_CRC_ENABLE_MASK 0x00000004
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATASTORE_CRC_ENABLE_SHIFT 2
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATASTORE_CRC_ENABLE_DISABLE 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_MISB_DATASTORE_CRC_ENABLE_ENABLE 1

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: LLMB_DATA_CRC_RESET [01:01] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_DATA_CRC_RESET_MASK 0x00000002
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_DATA_CRC_RESET_SHIFT 1
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_DATA_CRC_RESET_DISABLE 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_DATA_CRC_RESET_ENABLE 1

/* HIF_TOP_CTRL :: CRC_MISB_LLMB_CTRL :: LLMB_DATA_CRC_ENABLE [00:00] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_DATA_CRC_ENABLE_MASK 0x00000001
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_DATA_CRC_ENABLE_SHIFT 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_DATA_CRC_ENABLE_DISABLE 0
#define BCHP_HIF_TOP_CTRL_CRC_MISB_LLMB_CTRL_LLMB_DATA_CRC_ENABLE_ENABLE 1

/***************************************************************************
 *CRC_LLMB_DATA0 - HIF CRC LLMB result register0
 ***************************************************************************/
/* HIF_TOP_CTRL :: CRC_LLMB_DATA0 :: LLMB_DATA_CRC0 [31:00] */
#define BCHP_HIF_TOP_CTRL_CRC_LLMB_DATA0_LLMB_DATA_CRC0_MASK       0xffffffff
#define BCHP_HIF_TOP_CTRL_CRC_LLMB_DATA0_LLMB_DATA_CRC0_SHIFT      0

/***************************************************************************
 *CRC_LLMB_DATA1 - HIF CRC LLMB result register1
 ***************************************************************************/
/* HIF_TOP_CTRL :: CRC_LLMB_DATA1 :: LLMB_DATA_CRC1 [31:00] */
#define BCHP_HIF_TOP_CTRL_CRC_LLMB_DATA1_LLMB_DATA_CRC1_MASK       0xffffffff
#define BCHP_HIF_TOP_CTRL_CRC_LLMB_DATA1_LLMB_DATA_CRC1_SHIFT      0

/***************************************************************************
 *CRC_MISBSTORE_DATA0 - HIF CRC MISB store data bus result register0
 ***************************************************************************/
/* HIF_TOP_CTRL :: CRC_MISBSTORE_DATA0 :: MISB_DATASTORE_CRC0 [31:00] */
#define BCHP_HIF_TOP_CTRL_CRC_MISBSTORE_DATA0_MISB_DATASTORE_CRC0_MASK 0xffffffff
#define BCHP_HIF_TOP_CTRL_CRC_MISBSTORE_DATA0_MISB_DATASTORE_CRC0_SHIFT 0

/***************************************************************************
 *CRC_MISBSTORE_DATA1 - HIF CRC MISB store data bus result register1
 ***************************************************************************/
/* HIF_TOP_CTRL :: CRC_MISBSTORE_DATA1 :: MISB_DATASTORE_CRC1 [31:00] */
#define BCHP_HIF_TOP_CTRL_CRC_MISBSTORE_DATA1_MISB_DATASTORE_CRC1_MASK 0xffffffff
#define BCHP_HIF_TOP_CTRL_CRC_MISBSTORE_DATA1_MISB_DATASTORE_CRC1_SHIFT 0

/***************************************************************************
 *CRC_MISBLOAD_DATA0 - HIF CRC MISB store data bus result register0
 ***************************************************************************/
/* HIF_TOP_CTRL :: CRC_MISBLOAD_DATA0 :: MISB_DATALOAD_CRC0 [31:00] */
#define BCHP_HIF_TOP_CTRL_CRC_MISBLOAD_DATA0_MISB_DATALOAD_CRC0_MASK 0xffffffff
#define BCHP_HIF_TOP_CTRL_CRC_MISBLOAD_DATA0_MISB_DATALOAD_CRC0_SHIFT 0

/***************************************************************************
 *CRC_MISBLOAD_DATA1 - HIF CRC MISB store data bus result register1
 ***************************************************************************/
/* HIF_TOP_CTRL :: CRC_MISBLOAD_DATA1 :: MISB_DATALOAD_CRC1 [31:00] */
#define BCHP_HIF_TOP_CTRL_CRC_MISBLOAD_DATA1_MISB_DATALOAD_CRC1_MASK 0xffffffff
#define BCHP_HIF_TOP_CTRL_CRC_MISBLOAD_DATA1_MISB_DATALOAD_CRC1_SHIFT 0

/***************************************************************************
 *CRC_LLMB_ADDR - HIF CRC LLMB address bus result register
 ***************************************************************************/
/* HIF_TOP_CTRL :: CRC_LLMB_ADDR :: LLMB_ADDR_CRC [31:00] */
#define BCHP_HIF_TOP_CTRL_CRC_LLMB_ADDR_LLMB_ADDR_CRC_MASK         0xffffffff
#define BCHP_HIF_TOP_CTRL_CRC_LLMB_ADDR_LLMB_ADDR_CRC_SHIFT        0

/***************************************************************************
 *CRC_MISB_ADDR - HIF CRC MISB address bus result register
 ***************************************************************************/
/* HIF_TOP_CTRL :: CRC_MISB_ADDR :: MISB_ADDR_CRC [31:00] */
#define BCHP_HIF_TOP_CTRL_CRC_MISB_ADDR_MISB_ADDR_CRC_MASK         0xffffffff
#define BCHP_HIF_TOP_CTRL_CRC_MISB_ADDR_MISB_ADDR_CRC_SHIFT        0

/***************************************************************************
 *PCI_MWIN_CTRL - HIF PCI ConfigurationManager  MWIN Control Register
 ***************************************************************************/
/* HIF_TOP_CTRL :: PCI_MWIN_CTRL :: reserved0 [31:05] */
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL_reserved0_MASK             0xffffffe0
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL_reserved0_SHIFT            5

/* HIF_TOP_CTRL :: PCI_MWIN_CTRL :: MWIN2_EN [04:04] */
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL_MWIN2_EN_MASK              0x00000010
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL_MWIN2_EN_SHIFT             4

/* HIF_TOP_CTRL :: PCI_MWIN_CTRL :: MWIN1_EN [03:03] */
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL_MWIN1_EN_MASK              0x00000008
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL_MWIN1_EN_SHIFT             3

/* HIF_TOP_CTRL :: PCI_MWIN_CTRL :: reserved1 [02:02] */
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL_reserved1_MASK             0x00000004
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL_reserved1_SHIFT            2

/* HIF_TOP_CTRL :: PCI_MWIN_CTRL :: MWIN_SIZE [01:00] */
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL_MWIN_SIZE_MASK             0x00000003
#define BCHP_HIF_TOP_CTRL_PCI_MWIN_CTRL_MWIN_SIZE_SHIFT            0

#endif /* #ifndef BCHP_HIF_TOP_CTRL_H__ */

/* End of File */
