/****************************************************************************
 *     Copyright (c) 1999-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Fri Aug 15 15:20:54 2014
 *                 Full Compile MD5 Checksum  a68bc62e9dd3be19fcad480c369d60fd
 *                     (minus title and desc)
 *                 MD5 Checksum               14382795d76d8497c2dd1bcf3f5d36da
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     14541
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_STB_CHAN_CTRL_H__
#define BCHP_STB_CHAN_CTRL_H__

/***************************************************************************
 *STB_CHAN_CTRL
 ***************************************************************************/
#define BCHP_STB_CHAN_CTRL_REVID                 0x0121c000 /* [RO] REVID */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL             0x0121c004 /* [RW] XBAR Related Controls */
#define BCHP_STB_CHAN_CTRL_XBAR_IPATHTST         0x0121c008 /* [RO] XBAR Path Test */
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET        0x0121c00c /* [RW] Local Software Reset */
#define BCHP_STB_CHAN_CTRL_PWRDN                 0x0121c010 /* [RW] Channel Clock Gating Control */
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL           0x0121c014 /* [RW] Testport Out Controls */
#define BCHP_STB_CHAN_CTRL_TEST_FCW              0x0121c018 /* [RW] Test mixer frequency control word */
#define BCHP_STB_CHAN_CTRL_TEST_MIX              0x0121c01c /* [RW] Test mixer control */
#define BCHP_STB_CHAN_CTRL_TEST_TONE             0x0121c020 /* [RW] Test mixer tone output amplitude */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H0         0x0121c024 /* [RW] Test xsinx : filter coefficient tap 0 */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H1         0x0121c028 /* [RW] Test xsinx : filter coefficient tap 1 */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H2         0x0121c02c /* [RW] Test xsinx : filter coefficient tap 2 */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H3         0x0121c030 /* [RW] Test xsinx : filter coefficient tap 3 */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H4         0x0121c034 /* [RW] Test xsinx : filter coefficient tap 4 */
#define BCHP_STB_CHAN_CTRL_SW_SPARE0             0x0121c038 /* [RW] Spare Register For Software Test */
#define BCHP_STB_CHAN_CTRL_SW_SPARE1             0x0121c03c /* [RW] Spare Register For Software Test */

/***************************************************************************
 *REVID - REVID
 ***************************************************************************/
/* STB_CHAN_CTRL :: REVID :: reserved0 [31:16] */
#define BCHP_STB_CHAN_CTRL_REVID_reserved0_MASK                    0xffff0000
#define BCHP_STB_CHAN_CTRL_REVID_reserved0_SHIFT                   16

/* STB_CHAN_CTRL :: REVID :: MAJOR [15:08] */
#define BCHP_STB_CHAN_CTRL_REVID_MAJOR_MASK                        0x0000ff00
#define BCHP_STB_CHAN_CTRL_REVID_MAJOR_SHIFT                       8
#define BCHP_STB_CHAN_CTRL_REVID_MAJOR_DEFAULT                     0x00000001

/* STB_CHAN_CTRL :: REVID :: MINOR [07:00] */
#define BCHP_STB_CHAN_CTRL_REVID_MINOR_MASK                        0x000000ff
#define BCHP_STB_CHAN_CTRL_REVID_MINOR_SHIFT                       0
#define BCHP_STB_CHAN_CTRL_REVID_MINOR_DEFAULT                     0x00000000

/***************************************************************************
 *XBAR_CTRL - XBAR Related Controls
 ***************************************************************************/
/* STB_CHAN_CTRL :: XBAR_CTRL :: reserved0 [31:20] */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_reserved0_MASK                0xfff00000
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_reserved0_SHIFT               20

/* STB_CHAN_CTRL :: XBAR_CTRL :: ZERO_LSB [19:18] */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_ZERO_LSB_MASK                 0x000c0000
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_ZERO_LSB_SHIFT                18
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_ZERO_LSB_DEFAULT              0x00000000

/* STB_CHAN_CTRL :: XBAR_CTRL :: DCM_ON_REFADC [17:17] */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_ON_REFADC_MASK            0x00020000
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_ON_REFADC_SHIFT           17
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_ON_REFADC_DEFAULT         0x00000000

/* STB_CHAN_CTRL :: XBAR_CTRL :: DCM_ENBL [16:16] */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_ENBL_MASK                 0x00010000
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_ENBL_SHIFT                16
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_ENBL_DEFAULT              0x00000000

/* STB_CHAN_CTRL :: XBAR_CTRL :: DCM_LNSEL [15:12] */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_LNSEL_MASK                0x0000f000
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_LNSEL_SHIFT               12
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_LNSEL_DEFAULT             0x00000000

/* STB_CHAN_CTRL :: XBAR_CTRL :: DCM_RATIO [11:04] */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_RATIO_MASK                0x00000ff0
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_RATIO_SHIFT               4
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_DCM_RATIO_DEFAULT             0x00000020

/* STB_CHAN_CTRL :: XBAR_CTRL :: reserved_for_eco1 [03:03] */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_reserved_for_eco1_MASK        0x00000008
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_reserved_for_eco1_SHIFT       3
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_reserved_for_eco1_DEFAULT     0x00000000

/* STB_CHAN_CTRL :: XBAR_CTRL :: IPATHTST_EN [02:02] */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_IPATHTST_EN_MASK              0x00000004
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_IPATHTST_EN_SHIFT             2
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_IPATHTST_EN_DEFAULT           0x00000000

/* STB_CHAN_CTRL :: XBAR_CTRL :: LFSR_ON [01:01] */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_LFSR_ON_MASK                  0x00000002
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_LFSR_ON_SHIFT                 1
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_LFSR_ON_DEFAULT               0x00000000

/* STB_CHAN_CTRL :: XBAR_CTRL :: RFIFO_ON [00:00] */
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_RFIFO_ON_MASK                 0x00000001
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_RFIFO_ON_SHIFT                0
#define BCHP_STB_CHAN_CTRL_XBAR_CTRL_RFIFO_ON_DEFAULT              0x00000000

/***************************************************************************
 *XBAR_IPATHTST - XBAR Path Test
 ***************************************************************************/
/* STB_CHAN_CTRL :: XBAR_IPATHTST :: reserved0 [31:02] */
#define BCHP_STB_CHAN_CTRL_XBAR_IPATHTST_reserved0_MASK            0xfffffffc
#define BCHP_STB_CHAN_CTRL_XBAR_IPATHTST_reserved0_SHIFT           2

/* STB_CHAN_CTRL :: XBAR_IPATHTST :: STATUS [01:00] */
#define BCHP_STB_CHAN_CTRL_XBAR_IPATHTST_STATUS_MASK               0x00000003
#define BCHP_STB_CHAN_CTRL_XBAR_IPATHTST_STATUS_SHIFT              0
#define BCHP_STB_CHAN_CTRL_XBAR_IPATHTST_STATUS_DEFAULT            0x00000000

/***************************************************************************
 *LOCAL_SW_RESET - Local Software Reset
 ***************************************************************************/
/* STB_CHAN_CTRL :: LOCAL_SW_RESET :: reserved0 [31:03] */
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_reserved0_MASK           0xfffffff8
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_reserved0_SHIFT          3

/* STB_CHAN_CTRL :: LOCAL_SW_RESET :: TEST [02:02] */
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_TEST_MASK                0x00000004
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_TEST_SHIFT               2
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_TEST_DEFAULT             0x00000001

/* STB_CHAN_CTRL :: LOCAL_SW_RESET :: CHAN1 [01:01] */
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_CHAN1_MASK               0x00000002
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_CHAN1_SHIFT              1
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_CHAN1_DEFAULT            0x00000001

/* STB_CHAN_CTRL :: LOCAL_SW_RESET :: CHAN0 [00:00] */
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_CHAN0_MASK               0x00000001
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_CHAN0_SHIFT              0
#define BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET_CHAN0_DEFAULT            0x00000001

/***************************************************************************
 *PWRDN - Channel Clock Gating Control
 ***************************************************************************/
/* STB_CHAN_CTRL :: PWRDN :: reserved0 [31:03] */
#define BCHP_STB_CHAN_CTRL_PWRDN_reserved0_MASK                    0xfffffff8
#define BCHP_STB_CHAN_CTRL_PWRDN_reserved0_SHIFT                   3

/* STB_CHAN_CTRL :: PWRDN :: TEST [02:02] */
#define BCHP_STB_CHAN_CTRL_PWRDN_TEST_MASK                         0x00000004
#define BCHP_STB_CHAN_CTRL_PWRDN_TEST_SHIFT                        2
#define BCHP_STB_CHAN_CTRL_PWRDN_TEST_DEFAULT                      0x00000000

/* STB_CHAN_CTRL :: PWRDN :: CHAN1 [01:01] */
#define BCHP_STB_CHAN_CTRL_PWRDN_CHAN1_MASK                        0x00000002
#define BCHP_STB_CHAN_CTRL_PWRDN_CHAN1_SHIFT                       1
#define BCHP_STB_CHAN_CTRL_PWRDN_CHAN1_DEFAULT                     0x00000000

/* STB_CHAN_CTRL :: PWRDN :: CHAN0 [00:00] */
#define BCHP_STB_CHAN_CTRL_PWRDN_CHAN0_MASK                        0x00000001
#define BCHP_STB_CHAN_CTRL_PWRDN_CHAN0_SHIFT                       0
#define BCHP_STB_CHAN_CTRL_PWRDN_CHAN0_DEFAULT                     0x00000000

/***************************************************************************
 *TP_OUT_CTRL - Testport Out Controls
 ***************************************************************************/
/* STB_CHAN_CTRL :: TP_OUT_CTRL :: reserved0 [31:09] */
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_reserved0_MASK              0xfffffe00
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_reserved0_SHIFT             9

/* STB_CHAN_CTRL :: TP_OUT_CTRL :: DEC [08:08] */
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_DEC_MASK                    0x00000100
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_DEC_SHIFT                   8
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_DEC_DEFAULT                 0x00000000

/* STB_CHAN_CTRL :: TP_OUT_CTRL :: reserved_for_eco1 [07:05] */
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_reserved_for_eco1_MASK      0x000000e0
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_reserved_for_eco1_SHIFT     5
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_reserved_for_eco1_DEFAULT   0x00000000

/* STB_CHAN_CTRL :: TP_OUT_CTRL :: PRIMARY [04:04] */
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_PRIMARY_MASK                0x00000010
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_PRIMARY_SHIFT               4
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_PRIMARY_DEFAULT             0x00000000

/* STB_CHAN_CTRL :: TP_OUT_CTRL :: reserved_for_eco2 [03:01] */
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_reserved_for_eco2_MASK      0x0000000e
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_reserved_for_eco2_SHIFT     1
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_reserved_for_eco2_DEFAULT   0x00000000

/* STB_CHAN_CTRL :: TP_OUT_CTRL :: CHAN [00:00] */
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_CHAN_MASK                   0x00000001
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_CHAN_SHIFT                  0
#define BCHP_STB_CHAN_CTRL_TP_OUT_CTRL_CHAN_DEFAULT                0x00000000

/***************************************************************************
 *TEST_FCW - Test mixer frequency control word
 ***************************************************************************/
/* STB_CHAN_CTRL :: TEST_FCW :: reserved0 [31:28] */
#define BCHP_STB_CHAN_CTRL_TEST_FCW_reserved0_MASK                 0xf0000000
#define BCHP_STB_CHAN_CTRL_TEST_FCW_reserved0_SHIFT                28

/* STB_CHAN_CTRL :: TEST_FCW :: TEST_FCW [27:00] */
#define BCHP_STB_CHAN_CTRL_TEST_FCW_TEST_FCW_MASK                  0x0fffffff
#define BCHP_STB_CHAN_CTRL_TEST_FCW_TEST_FCW_SHIFT                 0
#define BCHP_STB_CHAN_CTRL_TEST_FCW_TEST_FCW_DEFAULT               0x00000000

/***************************************************************************
 *TEST_MIX - Test mixer control
 ***************************************************************************/
/* STB_CHAN_CTRL :: TEST_MIX :: reserved0 [31:02] */
#define BCHP_STB_CHAN_CTRL_TEST_MIX_reserved0_MASK                 0xfffffffc
#define BCHP_STB_CHAN_CTRL_TEST_MIX_reserved0_SHIFT                2

/* STB_CHAN_CTRL :: TEST_MIX :: TONE_SEL [01:01] */
#define BCHP_STB_CHAN_CTRL_TEST_MIX_TONE_SEL_MASK                  0x00000002
#define BCHP_STB_CHAN_CTRL_TEST_MIX_TONE_SEL_SHIFT                 1
#define BCHP_STB_CHAN_CTRL_TEST_MIX_TONE_SEL_DEFAULT               0x00000000

/* STB_CHAN_CTRL :: TEST_MIX :: SPEC_INV [00:00] */
#define BCHP_STB_CHAN_CTRL_TEST_MIX_SPEC_INV_MASK                  0x00000001
#define BCHP_STB_CHAN_CTRL_TEST_MIX_SPEC_INV_SHIFT                 0
#define BCHP_STB_CHAN_CTRL_TEST_MIX_SPEC_INV_DEFAULT               0x00000000

/***************************************************************************
 *TEST_TONE - Test mixer tone output amplitude
 ***************************************************************************/
/* STB_CHAN_CTRL :: TEST_TONE :: reserved0 [31:27] */
#define BCHP_STB_CHAN_CTRL_TEST_TONE_reserved0_MASK                0xf8000000
#define BCHP_STB_CHAN_CTRL_TEST_TONE_reserved0_SHIFT               27

/* STB_CHAN_CTRL :: TEST_TONE :: AMPL_Q [26:16] */
#define BCHP_STB_CHAN_CTRL_TEST_TONE_AMPL_Q_MASK                   0x07ff0000
#define BCHP_STB_CHAN_CTRL_TEST_TONE_AMPL_Q_SHIFT                  16
#define BCHP_STB_CHAN_CTRL_TEST_TONE_AMPL_Q_DEFAULT                0x00000000

/* STB_CHAN_CTRL :: TEST_TONE :: reserved_for_eco1 [15:11] */
#define BCHP_STB_CHAN_CTRL_TEST_TONE_reserved_for_eco1_MASK        0x0000f800
#define BCHP_STB_CHAN_CTRL_TEST_TONE_reserved_for_eco1_SHIFT       11
#define BCHP_STB_CHAN_CTRL_TEST_TONE_reserved_for_eco1_DEFAULT     0x00000000

/* STB_CHAN_CTRL :: TEST_TONE :: AMPL_I [10:00] */
#define BCHP_STB_CHAN_CTRL_TEST_TONE_AMPL_I_MASK                   0x000007ff
#define BCHP_STB_CHAN_CTRL_TEST_TONE_AMPL_I_SHIFT                  0
#define BCHP_STB_CHAN_CTRL_TEST_TONE_AMPL_I_DEFAULT                0x00000000

/***************************************************************************
 *TEST_XSINX_H0 - Test xsinx : filter coefficient tap 0
 ***************************************************************************/
/* STB_CHAN_CTRL :: TEST_XSINX_H0 :: reserved0 [31:16] */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H0_reserved0_MASK            0xffff0000
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H0_reserved0_SHIFT           16

/* STB_CHAN_CTRL :: TEST_XSINX_H0 :: TEST_XSINX_H0 [15:00] */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H0_TEST_XSINX_H0_MASK        0x0000ffff
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H0_TEST_XSINX_H0_SHIFT       0
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H0_TEST_XSINX_H0_DEFAULT     0x00000000

/***************************************************************************
 *TEST_XSINX_H1 - Test xsinx : filter coefficient tap 1
 ***************************************************************************/
/* STB_CHAN_CTRL :: TEST_XSINX_H1 :: reserved0 [31:16] */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H1_reserved0_MASK            0xffff0000
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H1_reserved0_SHIFT           16

/* STB_CHAN_CTRL :: TEST_XSINX_H1 :: TEST_XSINX_H1 [15:00] */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H1_TEST_XSINX_H1_MASK        0x0000ffff
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H1_TEST_XSINX_H1_SHIFT       0
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H1_TEST_XSINX_H1_DEFAULT     0x00000000

/***************************************************************************
 *TEST_XSINX_H2 - Test xsinx : filter coefficient tap 2
 ***************************************************************************/
/* STB_CHAN_CTRL :: TEST_XSINX_H2 :: reserved0 [31:16] */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H2_reserved0_MASK            0xffff0000
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H2_reserved0_SHIFT           16

/* STB_CHAN_CTRL :: TEST_XSINX_H2 :: TEST_XSINX_H2 [15:00] */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H2_TEST_XSINX_H2_MASK        0x0000ffff
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H2_TEST_XSINX_H2_SHIFT       0
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H2_TEST_XSINX_H2_DEFAULT     0x00000000

/***************************************************************************
 *TEST_XSINX_H3 - Test xsinx : filter coefficient tap 3
 ***************************************************************************/
/* STB_CHAN_CTRL :: TEST_XSINX_H3 :: reserved0 [31:16] */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H3_reserved0_MASK            0xffff0000
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H3_reserved0_SHIFT           16

/* STB_CHAN_CTRL :: TEST_XSINX_H3 :: TEST_XSINX_H0 [15:00] */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H3_TEST_XSINX_H0_MASK        0x0000ffff
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H3_TEST_XSINX_H0_SHIFT       0
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H3_TEST_XSINX_H0_DEFAULT     0x00000000

/***************************************************************************
 *TEST_XSINX_H4 - Test xsinx : filter coefficient tap 4
 ***************************************************************************/
/* STB_CHAN_CTRL :: TEST_XSINX_H4 :: reserved0 [31:16] */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H4_reserved0_MASK            0xffff0000
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H4_reserved0_SHIFT           16

/* STB_CHAN_CTRL :: TEST_XSINX_H4 :: TEST_XSINX_H0 [15:00] */
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H4_TEST_XSINX_H0_MASK        0x0000ffff
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H4_TEST_XSINX_H0_SHIFT       0
#define BCHP_STB_CHAN_CTRL_TEST_XSINX_H4_TEST_XSINX_H0_DEFAULT     0x00000000

/***************************************************************************
 *SW_SPARE0 - Spare Register For Software Test
 ***************************************************************************/
/* STB_CHAN_CTRL :: SW_SPARE0 :: SW_SPARE0 [31:00] */
#define BCHP_STB_CHAN_CTRL_SW_SPARE0_SW_SPARE0_MASK                0xffffffff
#define BCHP_STB_CHAN_CTRL_SW_SPARE0_SW_SPARE0_SHIFT               0
#define BCHP_STB_CHAN_CTRL_SW_SPARE0_SW_SPARE0_DEFAULT             0x00000000

/***************************************************************************
 *SW_SPARE1 - Spare Register For Software Test
 ***************************************************************************/
/* STB_CHAN_CTRL :: SW_SPARE1 :: SW_SPARE1 [31:00] */
#define BCHP_STB_CHAN_CTRL_SW_SPARE1_SW_SPARE1_MASK                0xffffffff
#define BCHP_STB_CHAN_CTRL_SW_SPARE1_SW_SPARE1_SHIFT               0
#define BCHP_STB_CHAN_CTRL_SW_SPARE1_SW_SPARE1_DEFAULT             0x00000000

#endif /* #ifndef BCHP_STB_CHAN_CTRL_H__ */

/* End of File */
