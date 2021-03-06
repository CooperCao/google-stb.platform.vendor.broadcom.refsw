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
 * Date:           Generated on         Fri Jan 22 20:12:35 2010
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

#ifndef BCHP_DS_TUNER_DIGCTL_0_0_H__
#define BCHP_DS_TUNER_DIGCTL_0_0_H__

/***************************************************************************
 *DS_TUNER_DIGCTL_0_0 - Tuner Digital Circuit Control  Registers 0 0
 ***************************************************************************/
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14       0x04c34550 /* Tuner VGA DCO Hystersis Control Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15       0x04c34554 /* Tuner VGA DCO Error Micro Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_16       0x04c34558 /* Tuner VGA DCO Error Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17       0x04c3455c /* Tuner VGA DCO Control Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_18       0x04c34560 /* Tuner VGA DCO Integrator I Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_19       0x04c34564 /* Tuner VGA DCO Integrator Q Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20       0x04c34600 /* Tuner Mixer DCO Control Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_21       0x04c34604 /* Tuner Global Freeze Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_22       0x04c34608 /* Tuner Mixer DCO Loop Filter Output I */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_23       0x04c3460c /* Tuner Mixer DCO Loop Filter Output Q */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_24       0x04c34610 /* Tuner Mixer DCO Loop Filter Integrator I, 32 LSB */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_25       0x04c34614 /* Tuner Mixer DCO Loop Filter Integrator I, 32 MSB */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_26       0x04c34618 /* Tuner Mixer DCO Loop Filter Integrator Q, 32 LSB */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_27       0x04c3461c /* Tuner Mixer DCO Loop Filter Integrator Q, 32 MSB */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_28       0x04c34620 /* Tuner Mixer DCO Delta Sigma Converter Integrator I */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_29       0x04c34624 /* Tuner Mixer DCO Delta Sigma Converter Integrator Q */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_0E       0x04c34680 /* Digital Tuner Control Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_0F       0x04c34684 /* Tuner LO DDFS FCW Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30       0x04c34688 /* Tuner DDFS control Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40       0x04c34700 /* Power Down and Reset Control Register */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41       0x04c34704 /* clock Control Register */

/***************************************************************************
 *DIGCTL_14 - Tuner VGA DCO Hystersis Control Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: reserved_for_eco0 [31:16] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_reserved_for_eco0_MASK  0xffff0000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_reserved_for_eco0_SHIFT 16

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: reserved1 [15:13] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_reserved1_MASK          0x0000e000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_reserved1_SHIFT         13

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: VGA_DCO_TPIN_SEL [12:12] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_TPIN_SEL_MASK   0x00001000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_TPIN_SEL_SHIFT  12

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: reserved2 [11:11] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_reserved2_MASK          0x00000800
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_reserved2_SHIFT         11

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: VGA_DCO_OUT_EDGE_SEL [10:10] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_OUT_EDGE_SEL_MASK 0x00000400
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_OUT_EDGE_SEL_SHIFT 10

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: VGA_DCO_GAIN_LIMIT_ON [09:09] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_GAIN_LIMIT_ON_MASK 0x00000200
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_GAIN_LIMIT_ON_SHIFT 9

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: VGA_DCO_TWOS_COMP [08:08] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_TWOS_COMP_MASK  0x00000100
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_TWOS_COMP_SHIFT 8

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: VGA_DCO_OUT_INV_EN_Q [07:07] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_OUT_INV_EN_Q_MASK 0x00000080
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_OUT_INV_EN_Q_SHIFT 7

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: VGA_DCO_OUT_INV_EN_I [06:06] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_OUT_INV_EN_I_MASK 0x00000040
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_OUT_INV_EN_I_SHIFT 6

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: VGA_DCO_OUT_FRZ [05:05] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_OUT_FRZ_MASK    0x00000020
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_OUT_FRZ_SHIFT   5

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: VGA_DCO_HYS_DEC_SEL [04:04] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_HYS_DEC_SEL_MASK 0x00000010
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_HYS_DEC_SEL_SHIFT 4

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_14 :: VGA_DCO_HYS_DEC_RATE [03:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_HYS_DEC_RATE_MASK 0x0000000f
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_14_VGA_DCO_HYS_DEC_RATE_SHIFT 0

/***************************************************************************
 *DIGCTL_15 - Tuner VGA DCO Error Micro Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_15 :: USE_MICRO_VGA_DCO [31:31] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15_USE_MICRO_VGA_DCO_MASK  0x80000000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15_USE_MICRO_VGA_DCO_SHIFT 31

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_15 :: reserved0 [30:13] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15_reserved0_MASK          0x7fffe000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15_reserved0_SHIFT         13

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_15 :: VGA_DCO_ERR_Q_MICRO [12:08] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15_VGA_DCO_ERR_Q_MICRO_MASK 0x00001f00
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15_VGA_DCO_ERR_Q_MICRO_SHIFT 8

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_15 :: reserved_for_eco1 [07:05] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15_reserved_for_eco1_MASK  0x000000e0
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15_reserved_for_eco1_SHIFT 5

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_15 :: VGA_DCO_ERR_I_MICRO [04:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15_VGA_DCO_ERR_I_MICRO_MASK 0x0000001f
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_15_VGA_DCO_ERR_I_MICRO_SHIFT 0

/***************************************************************************
 *DIGCTL_16 - Tuner VGA DCO Error Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_16 :: reserved0 [31:13] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_16_reserved0_MASK          0xffffe000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_16_reserved0_SHIFT         13

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_16 :: VGA_DCO_ERR_Q [12:08] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_16_VGA_DCO_ERR_Q_MASK      0x00001f00
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_16_VGA_DCO_ERR_Q_SHIFT     8

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_16 :: reserved1 [07:05] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_16_reserved1_MASK          0x000000e0
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_16_reserved1_SHIFT         5

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_16 :: VGA_DCO_ERR_I [04:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_16_VGA_DCO_ERR_I_MASK      0x0000001f
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_16_VGA_DCO_ERR_I_SHIFT     0

/***************************************************************************
 *DIGCTL_17 - Tuner VGA DCO Control Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_17 :: VGA_DCO_TPIN_SEL [31:31] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_TPIN_SEL_MASK   0x80000000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_TPIN_SEL_SHIFT  31

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_17 :: reserved0 [30:17] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_reserved0_MASK          0x7ffe0000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_reserved0_SHIFT         17

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_17 :: VGA_DCO_OUT_SEL [16:12] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_OUT_SEL_MASK    0x0001f000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_OUT_SEL_SHIFT   12

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_17 :: VGA_DCO_IN_INV_EN_Q [11:11] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_IN_INV_EN_Q_MASK 0x00000800
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_IN_INV_EN_Q_SHIFT 11

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_17 :: VGA_DCO_IN_INV_EN_I [10:10] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_IN_INV_EN_I_MASK 0x00000400
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_IN_INV_EN_I_SHIFT 10

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_17 :: VGA_DCO_IN_DESTAG [09:09] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_IN_DESTAG_MASK  0x00000200
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_IN_DESTAG_SHIFT 9

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_17 :: VGA_DCO_BWC [08:04] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_BWC_MASK        0x000001f0
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_BWC_SHIFT       4

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_17 :: reserved1 [03:02] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_reserved1_MASK          0x0000000c
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_reserved1_SHIFT         2

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_17 :: VGA_DCO_RST [01:01] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_RST_MASK        0x00000002
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_RST_SHIFT       1
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_RST_on          1
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_RST_off         0

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_17 :: VGA_DCO_FRZ [00:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_FRZ_MASK        0x00000001
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_FRZ_SHIFT       0
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_FRZ_on          1
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17_VGA_DCO_FRZ_off         0

/***************************************************************************
 *DIGCTL_18 - Tuner VGA DCO Integrator I Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_18 :: VGA_DCO_INT_I [31:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_18_VGA_DCO_INT_I_MASK      0xffffffff
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_18_VGA_DCO_INT_I_SHIFT     0

/***************************************************************************
 *DIGCTL_19 - Tuner VGA DCO Integrator Q Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_19 :: VGA_DCO_INT_Q [31:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_19_VGA_DCO_INT_Q_MASK      0xffffffff
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_19_VGA_DCO_INT_Q_SHIFT     0

/***************************************************************************
 *DIGCTL_20 - Tuner Mixer DCO Control Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: DCOC_CLK_DIV [31:29] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_DCOC_CLK_DIV_MASK       0xe0000000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_DCOC_CLK_DIV_SHIFT      29

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: reserved_for_eco0 [28:24] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_reserved_for_eco0_MASK  0x1f000000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_reserved_for_eco0_SHIFT 24

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_INT_COEF [23:20] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_INT_COEF_MASK   0x00f00000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_INT_COEF_SHIFT  20

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: reserved1 [19:17] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_reserved1_MASK          0x000e0000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_reserved1_SHIFT         17

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_LIN_COEF [16:12] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_LIN_COEF_MASK   0x0001f000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_LIN_COEF_SHIFT  12

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: reserved2 [11:11] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_reserved2_MASK          0x00000800
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_reserved2_SHIFT         11

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_OUT_EDGE_SEL [10:10] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_OUT_EDGE_SEL_MASK 0x00000400
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_OUT_EDGE_SEL_SHIFT 10

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_OUT_INV_EN_Q [09:09] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_OUT_INV_EN_Q_MASK 0x00000200
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_OUT_INV_EN_Q_SHIFT 9

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_OUT_INV_EN_I [08:08] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_OUT_INV_EN_I_MASK 0x00000100
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_OUT_INV_EN_I_SHIFT 8

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_ADC_BYP_EN [07:07] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_ADC_BYP_EN_MASK 0x00000080
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_ADC_BYP_EN_SHIFT 7

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_IN_SEL [06:06] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_IN_SEL_MASK     0x00000040
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_IN_SEL_SHIFT    6

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: reserved3 [05:05] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_reserved3_MASK          0x00000020
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_reserved3_SHIFT         5

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_UPDATE_ADC_RATE [04:04] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_UPDATE_ADC_RATE_MASK 0x00000010
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_UPDATE_ADC_RATE_SHIFT 4

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_IN_EDGE_SEL [03:03] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_IN_EDGE_SEL_MASK 0x00000008
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_IN_EDGE_SEL_SHIFT 3

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_IN_INV_EN_Q [02:02] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_IN_INV_EN_Q_MASK 0x00000004
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_IN_INV_EN_Q_SHIFT 2

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_IN_INV_EN_I [01:01] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_IN_INV_EN_I_MASK 0x00000002
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_IN_INV_EN_I_SHIFT 1

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_20 :: MXR_DCO_OUT_FRZ [00:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_OUT_FRZ_MASK    0x00000001
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20_MXR_DCO_OUT_FRZ_SHIFT   0

/***************************************************************************
 *DIGCTL_21 - Tuner Global Freeze Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_21 :: reserved0 [31:01] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_21_reserved0_MASK          0xfffffffe
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_21_reserved0_SHIFT         1

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_21 :: MXR_DCO_GBL_FRZ [00:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_21_MXR_DCO_GBL_FRZ_MASK    0x00000001
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_21_MXR_DCO_GBL_FRZ_SHIFT   0

/***************************************************************************
 *DIGCTL_22 - Tuner Mixer DCO Loop Filter Output I
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_22 :: MXR_DCO_LP_I [31:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_22_MXR_DCO_LP_I_MASK       0xffffffff
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_22_MXR_DCO_LP_I_SHIFT      0

/***************************************************************************
 *DIGCTL_23 - Tuner Mixer DCO Loop Filter Output Q
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_23 :: MXR_DCO_LP_Q [31:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_23_MXR_DCO_LP_Q_MASK       0xffffffff
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_23_MXR_DCO_LP_Q_SHIFT      0

/***************************************************************************
 *DIGCTL_24 - Tuner Mixer DCO Loop Filter Integrator I, 32 LSB
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_24 :: MXR_DCO_LP_INT_LSB_I [31:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_24_MXR_DCO_LP_INT_LSB_I_MASK 0xffffffff
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_24_MXR_DCO_LP_INT_LSB_I_SHIFT 0

/***************************************************************************
 *DIGCTL_25 - Tuner Mixer DCO Loop Filter Integrator I, 32 MSB
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_25 :: MXR_DCO_LP_INT_MSB_I [31:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_25_MXR_DCO_LP_INT_MSB_I_MASK 0xffffffff
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_25_MXR_DCO_LP_INT_MSB_I_SHIFT 0

/***************************************************************************
 *DIGCTL_26 - Tuner Mixer DCO Loop Filter Integrator Q, 32 LSB
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_26 :: MXR_DCO_LP_INT_LSB_Q [31:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_26_MXR_DCO_LP_INT_LSB_Q_MASK 0xffffffff
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_26_MXR_DCO_LP_INT_LSB_Q_SHIFT 0

/***************************************************************************
 *DIGCTL_27 - Tuner Mixer DCO Loop Filter Integrator Q, 32 MSB
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_27 :: MXR_DCO_LP_INT_MSB_Q [31:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_27_MXR_DCO_LP_INT_MSB_Q_MASK 0xffffffff
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_27_MXR_DCO_LP_INT_MSB_Q_SHIFT 0

/***************************************************************************
 *DIGCTL_28 - Tuner Mixer DCO Delta Sigma Converter Integrator I
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_28 :: reserved0 [31:23] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_28_reserved0_MASK          0xff800000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_28_reserved0_SHIFT         23

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_28 :: MXR_DCO_DELSIG_INT_I [22:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_28_MXR_DCO_DELSIG_INT_I_MASK 0x007fffff
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_28_MXR_DCO_DELSIG_INT_I_SHIFT 0

/***************************************************************************
 *DIGCTL_29 - Tuner Mixer DCO Delta Sigma Converter Integrator Q
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_29 :: reserved0 [31:23] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_29_reserved0_MASK          0xff800000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_29_reserved0_SHIFT         23

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_29 :: MXR_DCO_DELSIG_INT_Q [22:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_29_MXR_DCO_DELSIG_INT_Q_MASK 0x007fffff
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_29_MXR_DCO_DELSIG_INT_Q_SHIFT 0

/***************************************************************************
 *DIGCTL_0E - Digital Tuner Control Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_0E :: LO_LF [31:03] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_0E_LO_LF_MASK              0xfffffff8
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_0E_LO_LF_SHIFT             3

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_0E :: reserved_for_eco0 [02:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_0E_reserved_for_eco0_MASK  0x00000007
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_0E_reserved_for_eco0_SHIFT 0

/***************************************************************************
 *DIGCTL_0F - Tuner LO DDFS FCW Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_0F :: FCW [31:04] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_0F_FCW_MASK                0xfffffff0
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_0F_FCW_SHIFT               4

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_0F :: reserved0 [03:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_0F_reserved0_MASK          0x0000000f
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_0F_reserved0_SHIFT         0

/***************************************************************************
 *DIGCTL_30 - Tuner DDFS control Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_30 :: LF_SHIFT_CTL [31:29] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_LF_SHIFT_CTL_MASK       0xe0000000
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_LF_SHIFT_CTL_SHIFT      29

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_30 :: reserved_for_eco0 [28:08] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_reserved_for_eco0_MASK  0x1fffff00
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_reserved_for_eco0_SHIFT 8

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_30 :: LE_SEL [07:06] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_LE_SEL_MASK             0x000000c0
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_LE_SEL_SHIFT            6

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_30 :: DDFS_TP_EN [05:05] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_DDFS_TP_EN_MASK         0x00000020
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_DDFS_TP_EN_SHIFT        5

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_30 :: TUNER_CL_EN [04:04] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_TUNER_CL_EN_MASK        0x00000010
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_TUNER_CL_EN_SHIFT       4

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_30 :: TUNER_CL_INV [03:03] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_TUNER_CL_INV_MASK       0x00000008
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_TUNER_CL_INV_SHIFT      3

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_30 :: reserved_for_eco1 [02:01] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_reserved_for_eco1_MASK  0x00000006
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_reserved_for_eco1_SHIFT 1

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_30 :: DDFS_TWOS_COMP_EN [00:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_DDFS_TWOS_COMP_EN_MASK  0x00000001
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_30_DDFS_TWOS_COMP_EN_SHIFT 0

/***************************************************************************
 *DIGCTL_40 - Power Down and Reset Control Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_40 :: reserved_for_eco0 [31:08] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_reserved_for_eco0_MASK  0xffffff00
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_reserved_for_eco0_SHIFT 8

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_40 :: DDFS_CORE_SRST [07:07] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_DDFS_CORE_SRST_MASK     0x00000080
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_DDFS_CORE_SRST_SHIFT    7

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_40 :: reserved_for_eco1 [06:03] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_reserved_for_eco1_MASK  0x00000078
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_reserved_for_eco1_SHIFT 3

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_40 :: CLK_QDSAFE_EN [02:02] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_CLK_QDSAFE_EN_MASK      0x00000004
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_CLK_QDSAFE_EN_SHIFT     2

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_40 :: CLK_VGA_DCO_EN [01:01] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_CLK_VGA_DCO_EN_MASK     0x00000002
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_CLK_VGA_DCO_EN_SHIFT    1

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_40 :: CLK_MXR_DCO_EN [00:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_CLK_MXR_DCO_EN_MASK     0x00000001
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_40_CLK_MXR_DCO_EN_SHIFT    0

/***************************************************************************
 *DIGCTL_41 - clock Control Register
 ***************************************************************************/
/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_41 :: reserved0 [31:05] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_reserved0_MASK          0xffffffe0
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_reserved0_SHIFT         5

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_41 :: DDFS_BYP_CLK_EN [04:04] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_DDFS_BYP_CLK_EN_MASK    0x00000010
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_DDFS_BYP_CLK_EN_SHIFT   4

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_41 :: reserved_for_eco1 [03:03] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_reserved_for_eco1_MASK  0x00000008
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_reserved_for_eco1_SHIFT 3

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_41 :: TEST_EN_CLK_QDSAFE [02:02] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_TEST_EN_CLK_QDSAFE_MASK 0x00000004
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_TEST_EN_CLK_QDSAFE_SHIFT 2

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_41 :: TEST_EN_CLK_VGA_DCO [01:01] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_TEST_EN_CLK_VGA_DCO_MASK 0x00000002
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_TEST_EN_CLK_VGA_DCO_SHIFT 1

/* DS_TUNER_DIGCTL_0_0 :: DIGCTL_41 :: TEST_EN_CLK_MXR_DCO [00:00] */
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_TEST_EN_CLK_MXR_DCO_MASK 0x00000001
#define BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_41_TEST_EN_CLK_MXR_DCO_SHIFT 0

#endif /* #ifndef BCHP_DS_TUNER_DIGCTL_0_0_H__ */

/* End of File */
