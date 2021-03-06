/***************************************************************************
 *     Copyright (c) 1999-2008, Broadcom Corporation
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
 * Date:           Generated on         Wed Jan  9 09:00:31 2008
 *                 MD5 Checksum         847dc12a9d71c4c68a648bbf19a883e3
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
#define BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL          0x00002400 /* External IRQ Active Level Control Register */
#define BCHP_HIF_TOP_CTRL_TM_CTRL                0x00002404 /* HIF MBIST_TM_CTRL Register */
#define BCHP_HIF_TOP_CTRL_SCRATCH                0x00002408 /* HIF Scratch Register */

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
/* HIF_TOP_CTRL :: TM_CTRL :: reserved0 [31:08] */
#define BCHP_HIF_TOP_CTRL_TM_CTRL_reserved0_MASK                   0xffffff00
#define BCHP_HIF_TOP_CTRL_TM_CTRL_reserved0_SHIFT                  8

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

#endif /* #ifndef BCHP_HIF_TOP_CTRL_H__ */

/* End of File */
