/***************************************************************************
 *     Copyright (c) 1999-2011, Broadcom Corporation
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
 * Date:           Generated on         Fri Jul  1 18:07:20 2011
 *                 MD5 Checksum         e403e647ebd6e61a7eb0825000ed4941
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

#ifndef BCHP_WBADC_AI_H__
#define BCHP_WBADC_AI_H__

/***************************************************************************
 *WBADC_AI - UFE 6-bit WDADC Analog Interface Registers for DCOs & AGCs
 ***************************************************************************/
#define BCHP_WBADC_AI_CTRL                       0x000f0400 /* Control register for ADC input */
#define BCHP_WBADC_AI_TP                         0x000f0404 /* TP control registers */
#define BCHP_WBADC_AI_LFSR_SEED                  0x000f0408 /* LFSR initial seed */
#define BCHP_WBADC_AI_DCOC0                      0x000f0410 /* DC Canceller Control Register */
#define BCHP_WBADC_AI_DCI0                       0x000f0414 /* DC Canceller Integrator Value */
#define BCHP_WBADC_AI_DCOC1                      0x000f0418 /* DC Canceller Control Register */
#define BCHP_WBADC_AI_DCI1                       0x000f041c /* DC Canceller Integrator Value */
#define BCHP_WBADC_AI_DCOC2                      0x000f0420 /* DC Canceller Control Register */
#define BCHP_WBADC_AI_DCI2                       0x000f0424 /* DC Canceller Integrator Value */
#define BCHP_WBADC_AI_DCOC3                      0x000f0428 /* DC Canceller Control Register */
#define BCHP_WBADC_AI_DCI3                       0x000f042c /* DC Canceller Integrator Value */
#define BCHP_WBADC_AI_AGC0                       0x000f0440 /* AGC Control Register */
#define BCHP_WBADC_AI_AGCI0                      0x000f0444 /* AGC Integrator Value */
#define BCHP_WBADC_AI_AGCL0                      0x000f0448 /* AGC Leaky Integrator Value */
#define BCHP_WBADC_AI_AGCTHRES0                  0x000f044c /* AGC Status Control Register */
#define BCHP_WBADC_AI_AGC1                       0x000f0450 /* AGC Control Register */
#define BCHP_WBADC_AI_AGCI1                      0x000f0454 /* AGC Integrator Value */
#define BCHP_WBADC_AI_AGCL1                      0x000f0458 /* AGC Leaky Integrator Value */
#define BCHP_WBADC_AI_AGCTHRES1                  0x000f045c /* AGC Status Control Register */
#define BCHP_WBADC_AI_AGC2                       0x000f0460 /* AGC Control Register */
#define BCHP_WBADC_AI_AGCI2                      0x000f0464 /* AGC Integrator Value */
#define BCHP_WBADC_AI_AGCL2                      0x000f0468 /* AGC Leaky Integrator Value */
#define BCHP_WBADC_AI_AGCTHRES2                  0x000f046c /* AGC Status Control Register */
#define BCHP_WBADC_AI_AGC3                       0x000f0470 /* AGC Control Register */
#define BCHP_WBADC_AI_AGCI3                      0x000f0474 /* AGC Integrator Value */
#define BCHP_WBADC_AI_AGCL3                      0x000f0478 /* AGC Leaky Integrator Value */
#define BCHP_WBADC_AI_AGCTHRES3                  0x000f047c /* AGC Status Control Register */

/***************************************************************************
 *CTRL - Control register for ADC input
 ***************************************************************************/
/* WBADC_AI :: CTRL :: INPUT_FMT [31:31] */
#define BCHP_WBADC_AI_CTRL_INPUT_FMT_MASK                          0x80000000
#define BCHP_WBADC_AI_CTRL_INPUT_FMT_SHIFT                         31
#define BCHP_WBADC_AI_CTRL_INPUT_FMT_DEFAULT                       0

/* WBADC_AI :: CTRL :: INPUT_EDGE [30:30] */
#define BCHP_WBADC_AI_CTRL_INPUT_EDGE_MASK                         0x40000000
#define BCHP_WBADC_AI_CTRL_INPUT_EDGE_SHIFT                        30
#define BCHP_WBADC_AI_CTRL_INPUT_EDGE_DEFAULT                      0

/* WBADC_AI :: CTRL :: reserved0 [29:23] */
#define BCHP_WBADC_AI_CTRL_reserved0_MASK                          0x3f800000
#define BCHP_WBADC_AI_CTRL_reserved0_SHIFT                         23

/* WBADC_AI :: CTRL :: INPUT270_SEL [22:20] */
#define BCHP_WBADC_AI_CTRL_INPUT270_SEL_MASK                       0x00700000
#define BCHP_WBADC_AI_CTRL_INPUT270_SEL_SHIFT                      20
#define BCHP_WBADC_AI_CTRL_INPUT270_SEL_DEFAULT                    0

/* WBADC_AI :: CTRL :: reserved1 [19:19] */
#define BCHP_WBADC_AI_CTRL_reserved1_MASK                          0x00080000
#define BCHP_WBADC_AI_CTRL_reserved1_SHIFT                         19

/* WBADC_AI :: CTRL :: INPUT180_SEL [18:16] */
#define BCHP_WBADC_AI_CTRL_INPUT180_SEL_MASK                       0x00070000
#define BCHP_WBADC_AI_CTRL_INPUT180_SEL_SHIFT                      16
#define BCHP_WBADC_AI_CTRL_INPUT180_SEL_DEFAULT                    0

/* WBADC_AI :: CTRL :: reserved2 [15:15] */
#define BCHP_WBADC_AI_CTRL_reserved2_MASK                          0x00008000
#define BCHP_WBADC_AI_CTRL_reserved2_SHIFT                         15

/* WBADC_AI :: CTRL :: INPUT90_SEL [14:12] */
#define BCHP_WBADC_AI_CTRL_INPUT90_SEL_MASK                        0x00007000
#define BCHP_WBADC_AI_CTRL_INPUT90_SEL_SHIFT                       12
#define BCHP_WBADC_AI_CTRL_INPUT90_SEL_DEFAULT                     0

/* WBADC_AI :: CTRL :: reserved3 [11:11] */
#define BCHP_WBADC_AI_CTRL_reserved3_MASK                          0x00000800
#define BCHP_WBADC_AI_CTRL_reserved3_SHIFT                         11

/* WBADC_AI :: CTRL :: INPUT0_SEL [10:08] */
#define BCHP_WBADC_AI_CTRL_INPUT0_SEL_MASK                         0x00000700
#define BCHP_WBADC_AI_CTRL_INPUT0_SEL_SHIFT                        8
#define BCHP_WBADC_AI_CTRL_INPUT0_SEL_DEFAULT                      0

/* WBADC_AI :: CTRL :: reserved_for_eco4 [07:00] */
#define BCHP_WBADC_AI_CTRL_reserved_for_eco4_MASK                  0x000000ff
#define BCHP_WBADC_AI_CTRL_reserved_for_eco4_SHIFT                 0
#define BCHP_WBADC_AI_CTRL_reserved_for_eco4_DEFAULT               0

/***************************************************************************
 *TP - TP control registers
 ***************************************************************************/
/* WBADC_AI :: TP :: reserved0 [31:31] */
#define BCHP_WBADC_AI_TP_reserved0_MASK                            0x80000000
#define BCHP_WBADC_AI_TP_reserved0_SHIFT                           31

/* WBADC_AI :: TP :: TPOUT_SEL [30:29] */
#define BCHP_WBADC_AI_TP_TPOUT_SEL_MASK                            0x60000000
#define BCHP_WBADC_AI_TP_TPOUT_SEL_SHIFT                           29
#define BCHP_WBADC_AI_TP_TPOUT_SEL_DEFAULT                         0

/* WBADC_AI :: TP :: reserved1 [28:12] */
#define BCHP_WBADC_AI_TP_reserved1_MASK                            0x1ffff000
#define BCHP_WBADC_AI_TP_reserved1_SHIFT                           12

/* WBADC_AI :: TP :: DIGISUM0_INC [11:10] */
#define BCHP_WBADC_AI_TP_DIGISUM0_INC_MASK                         0x00000c00
#define BCHP_WBADC_AI_TP_DIGISUM0_INC_SHIFT                        10
#define BCHP_WBADC_AI_TP_DIGISUM0_INC_DEFAULT                      0

/* WBADC_AI :: TP :: DIGISUM1_INC [09:08] */
#define BCHP_WBADC_AI_TP_DIGISUM1_INC_MASK                         0x00000300
#define BCHP_WBADC_AI_TP_DIGISUM1_INC_SHIFT                        8
#define BCHP_WBADC_AI_TP_DIGISUM1_INC_DEFAULT                      1

/* WBADC_AI :: TP :: AI_CHK [07:04] */
#define BCHP_WBADC_AI_TP_AI_CHK_MASK                               0x000000f0
#define BCHP_WBADC_AI_TP_AI_CHK_SHIFT                              4
#define BCHP_WBADC_AI_TP_AI_CHK_DEFAULT                            0

/* WBADC_AI :: TP :: reserved2 [03:02] */
#define BCHP_WBADC_AI_TP_reserved2_MASK                            0x0000000c
#define BCHP_WBADC_AI_TP_reserved2_SHIFT                           2

/* WBADC_AI :: TP :: LFSR_EN [01:01] */
#define BCHP_WBADC_AI_TP_LFSR_EN_MASK                              0x00000002
#define BCHP_WBADC_AI_TP_LFSR_EN_SHIFT                             1
#define BCHP_WBADC_AI_TP_LFSR_EN_DEFAULT                           0

/* WBADC_AI :: TP :: reserved3 [00:00] */
#define BCHP_WBADC_AI_TP_reserved3_MASK                            0x00000001
#define BCHP_WBADC_AI_TP_reserved3_SHIFT                           0

/***************************************************************************
 *LFSR_SEED - LFSR initial seed
 ***************************************************************************/
/* WBADC_AI :: LFSR_SEED :: SEED [31:00] */
#define BCHP_WBADC_AI_LFSR_SEED_SEED_MASK                          0xffffffff
#define BCHP_WBADC_AI_LFSR_SEED_SEED_SHIFT                         0
#define BCHP_WBADC_AI_LFSR_SEED_SEED_DEFAULT                       1

/***************************************************************************
 *DCOC0 - DC Canceller Control Register
 ***************************************************************************/
/* WBADC_AI :: DCOC0 :: reserved0 [31:08] */
#define BCHP_WBADC_AI_DCOC0_reserved0_MASK                         0xffffff00
#define BCHP_WBADC_AI_DCOC0_reserved0_SHIFT                        8

/* WBADC_AI :: DCOC0 :: DCOC_BWC [07:04] */
#define BCHP_WBADC_AI_DCOC0_DCOC_BWC_MASK                          0x000000f0
#define BCHP_WBADC_AI_DCOC0_DCOC_BWC_SHIFT                         4
#define BCHP_WBADC_AI_DCOC0_DCOC_BWC_DEFAULT                       8

/* WBADC_AI :: DCOC0 :: reserved1 [03:03] */
#define BCHP_WBADC_AI_DCOC0_reserved1_MASK                         0x00000008
#define BCHP_WBADC_AI_DCOC0_reserved1_SHIFT                        3

/* WBADC_AI :: DCOC0 :: DCOC_BYP [02:02] */
#define BCHP_WBADC_AI_DCOC0_DCOC_BYP_MASK                          0x00000004
#define BCHP_WBADC_AI_DCOC0_DCOC_BYP_SHIFT                         2
#define BCHP_WBADC_AI_DCOC0_DCOC_BYP_DEFAULT                       0

/* WBADC_AI :: DCOC0 :: DCOC_LPRST [01:01] */
#define BCHP_WBADC_AI_DCOC0_DCOC_LPRST_MASK                        0x00000002
#define BCHP_WBADC_AI_DCOC0_DCOC_LPRST_SHIFT                       1
#define BCHP_WBADC_AI_DCOC0_DCOC_LPRST_DEFAULT                     1

/* WBADC_AI :: DCOC0 :: DCOC_LPFRZ [00:00] */
#define BCHP_WBADC_AI_DCOC0_DCOC_LPFRZ_MASK                        0x00000001
#define BCHP_WBADC_AI_DCOC0_DCOC_LPFRZ_SHIFT                       0
#define BCHP_WBADC_AI_DCOC0_DCOC_LPFRZ_DEFAULT                     1

/***************************************************************************
 *DCI0 - DC Canceller Integrator Value
 ***************************************************************************/
/* WBADC_AI :: DCI0 :: DCVAL [31:00] */
#define BCHP_WBADC_AI_DCI0_DCVAL_MASK                              0xffffffff
#define BCHP_WBADC_AI_DCI0_DCVAL_SHIFT                             0
#define BCHP_WBADC_AI_DCI0_DCVAL_DEFAULT                           0

/***************************************************************************
 *DCOC1 - DC Canceller Control Register
 ***************************************************************************/
/* WBADC_AI :: DCOC1 :: reserved0 [31:08] */
#define BCHP_WBADC_AI_DCOC1_reserved0_MASK                         0xffffff00
#define BCHP_WBADC_AI_DCOC1_reserved0_SHIFT                        8

/* WBADC_AI :: DCOC1 :: DCOC_BWC [07:04] */
#define BCHP_WBADC_AI_DCOC1_DCOC_BWC_MASK                          0x000000f0
#define BCHP_WBADC_AI_DCOC1_DCOC_BWC_SHIFT                         4
#define BCHP_WBADC_AI_DCOC1_DCOC_BWC_DEFAULT                       8

/* WBADC_AI :: DCOC1 :: reserved1 [03:03] */
#define BCHP_WBADC_AI_DCOC1_reserved1_MASK                         0x00000008
#define BCHP_WBADC_AI_DCOC1_reserved1_SHIFT                        3

/* WBADC_AI :: DCOC1 :: DCOC_BYP [02:02] */
#define BCHP_WBADC_AI_DCOC1_DCOC_BYP_MASK                          0x00000004
#define BCHP_WBADC_AI_DCOC1_DCOC_BYP_SHIFT                         2
#define BCHP_WBADC_AI_DCOC1_DCOC_BYP_DEFAULT                       0

/* WBADC_AI :: DCOC1 :: DCOC_LPRST [01:01] */
#define BCHP_WBADC_AI_DCOC1_DCOC_LPRST_MASK                        0x00000002
#define BCHP_WBADC_AI_DCOC1_DCOC_LPRST_SHIFT                       1
#define BCHP_WBADC_AI_DCOC1_DCOC_LPRST_DEFAULT                     1

/* WBADC_AI :: DCOC1 :: DCOC_LPFRZ [00:00] */
#define BCHP_WBADC_AI_DCOC1_DCOC_LPFRZ_MASK                        0x00000001
#define BCHP_WBADC_AI_DCOC1_DCOC_LPFRZ_SHIFT                       0
#define BCHP_WBADC_AI_DCOC1_DCOC_LPFRZ_DEFAULT                     1

/***************************************************************************
 *DCI1 - DC Canceller Integrator Value
 ***************************************************************************/
/* WBADC_AI :: DCI1 :: DCVAL [31:00] */
#define BCHP_WBADC_AI_DCI1_DCVAL_MASK                              0xffffffff
#define BCHP_WBADC_AI_DCI1_DCVAL_SHIFT                             0
#define BCHP_WBADC_AI_DCI1_DCVAL_DEFAULT                           0

/***************************************************************************
 *DCOC2 - DC Canceller Control Register
 ***************************************************************************/
/* WBADC_AI :: DCOC2 :: reserved0 [31:08] */
#define BCHP_WBADC_AI_DCOC2_reserved0_MASK                         0xffffff00
#define BCHP_WBADC_AI_DCOC2_reserved0_SHIFT                        8

/* WBADC_AI :: DCOC2 :: DCOC_BWC [07:04] */
#define BCHP_WBADC_AI_DCOC2_DCOC_BWC_MASK                          0x000000f0
#define BCHP_WBADC_AI_DCOC2_DCOC_BWC_SHIFT                         4
#define BCHP_WBADC_AI_DCOC2_DCOC_BWC_DEFAULT                       8

/* WBADC_AI :: DCOC2 :: reserved1 [03:03] */
#define BCHP_WBADC_AI_DCOC2_reserved1_MASK                         0x00000008
#define BCHP_WBADC_AI_DCOC2_reserved1_SHIFT                        3

/* WBADC_AI :: DCOC2 :: DCOC_BYP [02:02] */
#define BCHP_WBADC_AI_DCOC2_DCOC_BYP_MASK                          0x00000004
#define BCHP_WBADC_AI_DCOC2_DCOC_BYP_SHIFT                         2
#define BCHP_WBADC_AI_DCOC2_DCOC_BYP_DEFAULT                       0

/* WBADC_AI :: DCOC2 :: DCOC_LPRST [01:01] */
#define BCHP_WBADC_AI_DCOC2_DCOC_LPRST_MASK                        0x00000002
#define BCHP_WBADC_AI_DCOC2_DCOC_LPRST_SHIFT                       1
#define BCHP_WBADC_AI_DCOC2_DCOC_LPRST_DEFAULT                     1

/* WBADC_AI :: DCOC2 :: DCOC_LPFRZ [00:00] */
#define BCHP_WBADC_AI_DCOC2_DCOC_LPFRZ_MASK                        0x00000001
#define BCHP_WBADC_AI_DCOC2_DCOC_LPFRZ_SHIFT                       0
#define BCHP_WBADC_AI_DCOC2_DCOC_LPFRZ_DEFAULT                     1

/***************************************************************************
 *DCI2 - DC Canceller Integrator Value
 ***************************************************************************/
/* WBADC_AI :: DCI2 :: DCVAL [31:00] */
#define BCHP_WBADC_AI_DCI2_DCVAL_MASK                              0xffffffff
#define BCHP_WBADC_AI_DCI2_DCVAL_SHIFT                             0
#define BCHP_WBADC_AI_DCI2_DCVAL_DEFAULT                           0

/***************************************************************************
 *DCOC3 - DC Canceller Control Register
 ***************************************************************************/
/* WBADC_AI :: DCOC3 :: reserved0 [31:08] */
#define BCHP_WBADC_AI_DCOC3_reserved0_MASK                         0xffffff00
#define BCHP_WBADC_AI_DCOC3_reserved0_SHIFT                        8

/* WBADC_AI :: DCOC3 :: DCOC_BWC [07:04] */
#define BCHP_WBADC_AI_DCOC3_DCOC_BWC_MASK                          0x000000f0
#define BCHP_WBADC_AI_DCOC3_DCOC_BWC_SHIFT                         4
#define BCHP_WBADC_AI_DCOC3_DCOC_BWC_DEFAULT                       8

/* WBADC_AI :: DCOC3 :: reserved1 [03:03] */
#define BCHP_WBADC_AI_DCOC3_reserved1_MASK                         0x00000008
#define BCHP_WBADC_AI_DCOC3_reserved1_SHIFT                        3

/* WBADC_AI :: DCOC3 :: DCOC_BYP [02:02] */
#define BCHP_WBADC_AI_DCOC3_DCOC_BYP_MASK                          0x00000004
#define BCHP_WBADC_AI_DCOC3_DCOC_BYP_SHIFT                         2
#define BCHP_WBADC_AI_DCOC3_DCOC_BYP_DEFAULT                       0

/* WBADC_AI :: DCOC3 :: DCOC_LPRST [01:01] */
#define BCHP_WBADC_AI_DCOC3_DCOC_LPRST_MASK                        0x00000002
#define BCHP_WBADC_AI_DCOC3_DCOC_LPRST_SHIFT                       1
#define BCHP_WBADC_AI_DCOC3_DCOC_LPRST_DEFAULT                     1

/* WBADC_AI :: DCOC3 :: DCOC_LPFRZ [00:00] */
#define BCHP_WBADC_AI_DCOC3_DCOC_LPFRZ_MASK                        0x00000001
#define BCHP_WBADC_AI_DCOC3_DCOC_LPFRZ_SHIFT                       0
#define BCHP_WBADC_AI_DCOC3_DCOC_LPFRZ_DEFAULT                     1

/***************************************************************************
 *DCI3 - DC Canceller Integrator Value
 ***************************************************************************/
/* WBADC_AI :: DCI3 :: DCVAL [31:00] */
#define BCHP_WBADC_AI_DCI3_DCVAL_MASK                              0xffffffff
#define BCHP_WBADC_AI_DCI3_DCVAL_SHIFT                             0
#define BCHP_WBADC_AI_DCI3_DCVAL_DEFAULT                           0

/***************************************************************************
 *AGC0 - AGC Control Register
 ***************************************************************************/
/* WBADC_AI :: AGC0 :: reserved0 [31:12] */
#define BCHP_WBADC_AI_AGC0_reserved0_MASK                          0xfffff000
#define BCHP_WBADC_AI_AGC0_reserved0_SHIFT                         12

/* WBADC_AI :: AGC0 :: K [11:08] */
#define BCHP_WBADC_AI_AGC0_K_MASK                                  0x00000f00
#define BCHP_WBADC_AI_AGC0_K_SHIFT                                 8
#define BCHP_WBADC_AI_AGC0_K_DEFAULT                               15

/* WBADC_AI :: AGC0 :: beta [07:04] */
#define BCHP_WBADC_AI_AGC0_beta_MASK                               0x000000f0
#define BCHP_WBADC_AI_AGC0_beta_SHIFT                              4
#define BCHP_WBADC_AI_AGC0_beta_DEFAULT                            4

/* WBADC_AI :: AGC0 :: reserved1 [03:03] */
#define BCHP_WBADC_AI_AGC0_reserved1_MASK                          0x00000008
#define BCHP_WBADC_AI_AGC0_reserved1_SHIFT                         3

/* WBADC_AI :: AGC0 :: BYP [02:02] */
#define BCHP_WBADC_AI_AGC0_BYP_MASK                                0x00000004
#define BCHP_WBADC_AI_AGC0_BYP_SHIFT                               2
#define BCHP_WBADC_AI_AGC0_BYP_DEFAULT                             0

/* WBADC_AI :: AGC0 :: AGCFRZ [01:01] */
#define BCHP_WBADC_AI_AGC0_AGCFRZ_MASK                             0x00000002
#define BCHP_WBADC_AI_AGC0_AGCFRZ_SHIFT                            1
#define BCHP_WBADC_AI_AGC0_AGCFRZ_DEFAULT                          1

/* WBADC_AI :: AGC0 :: AGCRST [00:00] */
#define BCHP_WBADC_AI_AGC0_AGCRST_MASK                             0x00000001
#define BCHP_WBADC_AI_AGC0_AGCRST_SHIFT                            0
#define BCHP_WBADC_AI_AGC0_AGCRST_DEFAULT                          1

/***************************************************************************
 *AGCI0 - AGC Integrator Value
 ***************************************************************************/
/* WBADC_AI :: AGCI0 :: AGCVAL [31:00] */
#define BCHP_WBADC_AI_AGCI0_AGCVAL_MASK                            0xffffffff
#define BCHP_WBADC_AI_AGCI0_AGCVAL_SHIFT                           0
#define BCHP_WBADC_AI_AGCI0_AGCVAL_DEFAULT                         1073741824

/***************************************************************************
 *AGCL0 - AGC Leaky Integrator Value
 ***************************************************************************/
/* WBADC_AI :: AGCL0 :: AGCLVAL [31:00] */
#define BCHP_WBADC_AI_AGCL0_AGCLVAL_MASK                           0xffffffff
#define BCHP_WBADC_AI_AGCL0_AGCLVAL_SHIFT                          0
#define BCHP_WBADC_AI_AGCL0_AGCLVAL_DEFAULT                        0

/***************************************************************************
 *AGCTHRES0 - AGC Status Control Register
 ***************************************************************************/
/* WBADC_AI :: AGCTHRES0 :: THRES_USE [31:31] */
#define BCHP_WBADC_AI_AGCTHRES0_THRES_USE_MASK                     0x80000000
#define BCHP_WBADC_AI_AGCTHRES0_THRES_USE_SHIFT                    31
#define BCHP_WBADC_AI_AGCTHRES0_THRES_USE_DEFAULT                  0

/* WBADC_AI :: AGCTHRES0 :: reserved0 [30:15] */
#define BCHP_WBADC_AI_AGCTHRES0_reserved0_MASK                     0x7fff8000
#define BCHP_WBADC_AI_AGCTHRES0_reserved0_SHIFT                    15

/* WBADC_AI :: AGCTHRES0 :: THRES [14:00] */
#define BCHP_WBADC_AI_AGCTHRES0_THRES_MASK                         0x00007fff
#define BCHP_WBADC_AI_AGCTHRES0_THRES_SHIFT                        0
#define BCHP_WBADC_AI_AGCTHRES0_THRES_DEFAULT                      0

/***************************************************************************
 *AGC1 - AGC Control Register
 ***************************************************************************/
/* WBADC_AI :: AGC1 :: reserved0 [31:12] */
#define BCHP_WBADC_AI_AGC1_reserved0_MASK                          0xfffff000
#define BCHP_WBADC_AI_AGC1_reserved0_SHIFT                         12

/* WBADC_AI :: AGC1 :: K [11:08] */
#define BCHP_WBADC_AI_AGC1_K_MASK                                  0x00000f00
#define BCHP_WBADC_AI_AGC1_K_SHIFT                                 8
#define BCHP_WBADC_AI_AGC1_K_DEFAULT                               15

/* WBADC_AI :: AGC1 :: beta [07:04] */
#define BCHP_WBADC_AI_AGC1_beta_MASK                               0x000000f0
#define BCHP_WBADC_AI_AGC1_beta_SHIFT                              4
#define BCHP_WBADC_AI_AGC1_beta_DEFAULT                            4

/* WBADC_AI :: AGC1 :: reserved1 [03:03] */
#define BCHP_WBADC_AI_AGC1_reserved1_MASK                          0x00000008
#define BCHP_WBADC_AI_AGC1_reserved1_SHIFT                         3

/* WBADC_AI :: AGC1 :: BYP [02:02] */
#define BCHP_WBADC_AI_AGC1_BYP_MASK                                0x00000004
#define BCHP_WBADC_AI_AGC1_BYP_SHIFT                               2
#define BCHP_WBADC_AI_AGC1_BYP_DEFAULT                             0

/* WBADC_AI :: AGC1 :: AGCFRZ [01:01] */
#define BCHP_WBADC_AI_AGC1_AGCFRZ_MASK                             0x00000002
#define BCHP_WBADC_AI_AGC1_AGCFRZ_SHIFT                            1
#define BCHP_WBADC_AI_AGC1_AGCFRZ_DEFAULT                          1

/* WBADC_AI :: AGC1 :: AGCRST [00:00] */
#define BCHP_WBADC_AI_AGC1_AGCRST_MASK                             0x00000001
#define BCHP_WBADC_AI_AGC1_AGCRST_SHIFT                            0
#define BCHP_WBADC_AI_AGC1_AGCRST_DEFAULT                          1

/***************************************************************************
 *AGCI1 - AGC Integrator Value
 ***************************************************************************/
/* WBADC_AI :: AGCI1 :: AGCVAL [31:00] */
#define BCHP_WBADC_AI_AGCI1_AGCVAL_MASK                            0xffffffff
#define BCHP_WBADC_AI_AGCI1_AGCVAL_SHIFT                           0
#define BCHP_WBADC_AI_AGCI1_AGCVAL_DEFAULT                         1073741824

/***************************************************************************
 *AGCL1 - AGC Leaky Integrator Value
 ***************************************************************************/
/* WBADC_AI :: AGCL1 :: AGCLVAL [31:00] */
#define BCHP_WBADC_AI_AGCL1_AGCLVAL_MASK                           0xffffffff
#define BCHP_WBADC_AI_AGCL1_AGCLVAL_SHIFT                          0
#define BCHP_WBADC_AI_AGCL1_AGCLVAL_DEFAULT                        0

/***************************************************************************
 *AGCTHRES1 - AGC Status Control Register
 ***************************************************************************/
/* WBADC_AI :: AGCTHRES1 :: THRES_USE [31:31] */
#define BCHP_WBADC_AI_AGCTHRES1_THRES_USE_MASK                     0x80000000
#define BCHP_WBADC_AI_AGCTHRES1_THRES_USE_SHIFT                    31
#define BCHP_WBADC_AI_AGCTHRES1_THRES_USE_DEFAULT                  0

/* WBADC_AI :: AGCTHRES1 :: reserved0 [30:15] */
#define BCHP_WBADC_AI_AGCTHRES1_reserved0_MASK                     0x7fff8000
#define BCHP_WBADC_AI_AGCTHRES1_reserved0_SHIFT                    15

/* WBADC_AI :: AGCTHRES1 :: THRES [14:00] */
#define BCHP_WBADC_AI_AGCTHRES1_THRES_MASK                         0x00007fff
#define BCHP_WBADC_AI_AGCTHRES1_THRES_SHIFT                        0
#define BCHP_WBADC_AI_AGCTHRES1_THRES_DEFAULT                      0

/***************************************************************************
 *AGC2 - AGC Control Register
 ***************************************************************************/
/* WBADC_AI :: AGC2 :: reserved0 [31:12] */
#define BCHP_WBADC_AI_AGC2_reserved0_MASK                          0xfffff000
#define BCHP_WBADC_AI_AGC2_reserved0_SHIFT                         12

/* WBADC_AI :: AGC2 :: K [11:08] */
#define BCHP_WBADC_AI_AGC2_K_MASK                                  0x00000f00
#define BCHP_WBADC_AI_AGC2_K_SHIFT                                 8
#define BCHP_WBADC_AI_AGC2_K_DEFAULT                               15

/* WBADC_AI :: AGC2 :: beta [07:04] */
#define BCHP_WBADC_AI_AGC2_beta_MASK                               0x000000f0
#define BCHP_WBADC_AI_AGC2_beta_SHIFT                              4
#define BCHP_WBADC_AI_AGC2_beta_DEFAULT                            4

/* WBADC_AI :: AGC2 :: reserved1 [03:03] */
#define BCHP_WBADC_AI_AGC2_reserved1_MASK                          0x00000008
#define BCHP_WBADC_AI_AGC2_reserved1_SHIFT                         3

/* WBADC_AI :: AGC2 :: BYP [02:02] */
#define BCHP_WBADC_AI_AGC2_BYP_MASK                                0x00000004
#define BCHP_WBADC_AI_AGC2_BYP_SHIFT                               2
#define BCHP_WBADC_AI_AGC2_BYP_DEFAULT                             0

/* WBADC_AI :: AGC2 :: AGCFRZ [01:01] */
#define BCHP_WBADC_AI_AGC2_AGCFRZ_MASK                             0x00000002
#define BCHP_WBADC_AI_AGC2_AGCFRZ_SHIFT                            1
#define BCHP_WBADC_AI_AGC2_AGCFRZ_DEFAULT                          1

/* WBADC_AI :: AGC2 :: AGCRST [00:00] */
#define BCHP_WBADC_AI_AGC2_AGCRST_MASK                             0x00000001
#define BCHP_WBADC_AI_AGC2_AGCRST_SHIFT                            0
#define BCHP_WBADC_AI_AGC2_AGCRST_DEFAULT                          1

/***************************************************************************
 *AGCI2 - AGC Integrator Value
 ***************************************************************************/
/* WBADC_AI :: AGCI2 :: AGCVAL [31:00] */
#define BCHP_WBADC_AI_AGCI2_AGCVAL_MASK                            0xffffffff
#define BCHP_WBADC_AI_AGCI2_AGCVAL_SHIFT                           0
#define BCHP_WBADC_AI_AGCI2_AGCVAL_DEFAULT                         1073741824

/***************************************************************************
 *AGCL2 - AGC Leaky Integrator Value
 ***************************************************************************/
/* WBADC_AI :: AGCL2 :: AGCLVAL [31:00] */
#define BCHP_WBADC_AI_AGCL2_AGCLVAL_MASK                           0xffffffff
#define BCHP_WBADC_AI_AGCL2_AGCLVAL_SHIFT                          0
#define BCHP_WBADC_AI_AGCL2_AGCLVAL_DEFAULT                        0

/***************************************************************************
 *AGCTHRES2 - AGC Status Control Register
 ***************************************************************************/
/* WBADC_AI :: AGCTHRES2 :: THRES_USE [31:31] */
#define BCHP_WBADC_AI_AGCTHRES2_THRES_USE_MASK                     0x80000000
#define BCHP_WBADC_AI_AGCTHRES2_THRES_USE_SHIFT                    31
#define BCHP_WBADC_AI_AGCTHRES2_THRES_USE_DEFAULT                  0

/* WBADC_AI :: AGCTHRES2 :: reserved0 [30:15] */
#define BCHP_WBADC_AI_AGCTHRES2_reserved0_MASK                     0x7fff8000
#define BCHP_WBADC_AI_AGCTHRES2_reserved0_SHIFT                    15

/* WBADC_AI :: AGCTHRES2 :: THRES [14:00] */
#define BCHP_WBADC_AI_AGCTHRES2_THRES_MASK                         0x00007fff
#define BCHP_WBADC_AI_AGCTHRES2_THRES_SHIFT                        0
#define BCHP_WBADC_AI_AGCTHRES2_THRES_DEFAULT                      0

/***************************************************************************
 *AGC3 - AGC Control Register
 ***************************************************************************/
/* WBADC_AI :: AGC3 :: reserved0 [31:12] */
#define BCHP_WBADC_AI_AGC3_reserved0_MASK                          0xfffff000
#define BCHP_WBADC_AI_AGC3_reserved0_SHIFT                         12

/* WBADC_AI :: AGC3 :: K [11:08] */
#define BCHP_WBADC_AI_AGC3_K_MASK                                  0x00000f00
#define BCHP_WBADC_AI_AGC3_K_SHIFT                                 8
#define BCHP_WBADC_AI_AGC3_K_DEFAULT                               15

/* WBADC_AI :: AGC3 :: beta [07:04] */
#define BCHP_WBADC_AI_AGC3_beta_MASK                               0x000000f0
#define BCHP_WBADC_AI_AGC3_beta_SHIFT                              4
#define BCHP_WBADC_AI_AGC3_beta_DEFAULT                            4

/* WBADC_AI :: AGC3 :: reserved1 [03:03] */
#define BCHP_WBADC_AI_AGC3_reserved1_MASK                          0x00000008
#define BCHP_WBADC_AI_AGC3_reserved1_SHIFT                         3

/* WBADC_AI :: AGC3 :: BYP [02:02] */
#define BCHP_WBADC_AI_AGC3_BYP_MASK                                0x00000004
#define BCHP_WBADC_AI_AGC3_BYP_SHIFT                               2
#define BCHP_WBADC_AI_AGC3_BYP_DEFAULT                             0

/* WBADC_AI :: AGC3 :: AGCFRZ [01:01] */
#define BCHP_WBADC_AI_AGC3_AGCFRZ_MASK                             0x00000002
#define BCHP_WBADC_AI_AGC3_AGCFRZ_SHIFT                            1
#define BCHP_WBADC_AI_AGC3_AGCFRZ_DEFAULT                          1

/* WBADC_AI :: AGC3 :: AGCRST [00:00] */
#define BCHP_WBADC_AI_AGC3_AGCRST_MASK                             0x00000001
#define BCHP_WBADC_AI_AGC3_AGCRST_SHIFT                            0
#define BCHP_WBADC_AI_AGC3_AGCRST_DEFAULT                          1

/***************************************************************************
 *AGCI3 - AGC Integrator Value
 ***************************************************************************/
/* WBADC_AI :: AGCI3 :: AGCVAL [31:00] */
#define BCHP_WBADC_AI_AGCI3_AGCVAL_MASK                            0xffffffff
#define BCHP_WBADC_AI_AGCI3_AGCVAL_SHIFT                           0
#define BCHP_WBADC_AI_AGCI3_AGCVAL_DEFAULT                         1073741824

/***************************************************************************
 *AGCL3 - AGC Leaky Integrator Value
 ***************************************************************************/
/* WBADC_AI :: AGCL3 :: AGCLVAL [31:00] */
#define BCHP_WBADC_AI_AGCL3_AGCLVAL_MASK                           0xffffffff
#define BCHP_WBADC_AI_AGCL3_AGCLVAL_SHIFT                          0
#define BCHP_WBADC_AI_AGCL3_AGCLVAL_DEFAULT                        0

/***************************************************************************
 *AGCTHRES3 - AGC Status Control Register
 ***************************************************************************/
/* WBADC_AI :: AGCTHRES3 :: THRES_USE [31:31] */
#define BCHP_WBADC_AI_AGCTHRES3_THRES_USE_MASK                     0x80000000
#define BCHP_WBADC_AI_AGCTHRES3_THRES_USE_SHIFT                    31
#define BCHP_WBADC_AI_AGCTHRES3_THRES_USE_DEFAULT                  0

/* WBADC_AI :: AGCTHRES3 :: reserved0 [30:15] */
#define BCHP_WBADC_AI_AGCTHRES3_reserved0_MASK                     0x7fff8000
#define BCHP_WBADC_AI_AGCTHRES3_reserved0_SHIFT                    15

/* WBADC_AI :: AGCTHRES3 :: THRES [14:00] */
#define BCHP_WBADC_AI_AGCTHRES3_THRES_MASK                         0x00007fff
#define BCHP_WBADC_AI_AGCTHRES3_THRES_SHIFT                        0
#define BCHP_WBADC_AI_AGCTHRES3_THRES_DEFAULT                      0

#endif /* #ifndef BCHP_WBADC_AI_H__ */

/* End of File */
