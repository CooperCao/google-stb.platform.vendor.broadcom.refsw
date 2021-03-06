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
 * Date:           Generated on               Mon Sep 15 10:12:22 2014
 *                 Full Compile MD5 Checksum  ef22086ebd4065e4fea50dbc64f17e5e
 *                     (minus title and desc)
 *                 MD5 Checksum               39fcae49037a6337517df43bfc24b21f
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     14796
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_DS_A_00_TRL_H__
#define BCHP_DS_A_00_TRL_H__

/***************************************************************************
 *DS_A_00_TRL - Downstream Timing Recovery Loop Registers
 ***************************************************************************/
#define BCHP_DS_A_00_TRL_TL_CTL                  0x02220400 /* [RW] Timing Loop Control Register */
#define BCHP_DS_A_00_TRL_TL_COEFFS               0x02220404 /* [RW] Timing Loop Coefficient Control Register */
#define BCHP_DS_A_00_TRL_TL_FCW                  0x02220408 /* [RW] Timing Loop Frequency Control Word Register */
#define BCHP_DS_A_00_TRL_TL_INT                  0x0222040c /* [RW] Timing Loop Integrator Register */
#define BCHP_DS_A_00_TRL_AGCB_CTL                0x02220410 /* [RW] Digital AGCB Control Register */
#define BCHP_DS_A_00_TRL_AGCB_COEFFS             0x02220414 /* [RW] Digital AGCB Coefficient Register */
#define BCHP_DS_A_00_TRL_AGCB_GACC               0x02220418 /* [RW] Digital AGCB Gain Accumulator Register */
#define BCHP_DS_A_00_TRL_US_FCW                  0x0222041c /* [RW] UpStream Interface Frequency Control Word */
#define BCHP_DS_A_00_TRL_US_TL_OFFSET            0x02220420 /* [RW] UpStream Interface Timing Offeset */

/***************************************************************************
 *TL_CTL - Timing Loop Control Register
 ***************************************************************************/
/* DS_A_00_TRL :: TL_CTL :: QAMSTS [31:31] */
#define BCHP_DS_A_00_TRL_TL_CTL_QAMSTS_MASK                        0x80000000
#define BCHP_DS_A_00_TRL_TL_CTL_QAMSTS_SHIFT                       31
#define BCHP_DS_A_00_TRL_TL_CTL_QAMSTS_DEFAULT                     0x00000000

/* DS_A_00_TRL :: TL_CTL :: FL_MASK [30:30] */
#define BCHP_DS_A_00_TRL_TL_CTL_FL_MASK_MASK                       0x40000000
#define BCHP_DS_A_00_TRL_TL_CTL_FL_MASK_SHIFT                      30
#define BCHP_DS_A_00_TRL_TL_CTL_FL_MASK_DEFAULT                    0x00000000

/* DS_A_00_TRL :: TL_CTL :: SL_MASK [29:29] */
#define BCHP_DS_A_00_TRL_TL_CTL_SL_MASK_MASK                       0x20000000
#define BCHP_DS_A_00_TRL_TL_CTL_SL_MASK_SHIFT                      29
#define BCHP_DS_A_00_TRL_TL_CTL_SL_MASK_DEFAULT                    0x00000000

/* DS_A_00_TRL :: TL_CTL :: ECO_SPARE_0 [28:05] */
#define BCHP_DS_A_00_TRL_TL_CTL_ECO_SPARE_0_MASK                   0x1fffffe0
#define BCHP_DS_A_00_TRL_TL_CTL_ECO_SPARE_0_SHIFT                  5
#define BCHP_DS_A_00_TRL_TL_CTL_ECO_SPARE_0_DEFAULT                0x00000000

/* DS_A_00_TRL :: TL_CTL :: PD_PREC [04:03] */
#define BCHP_DS_A_00_TRL_TL_CTL_PD_PREC_MASK                       0x00000018
#define BCHP_DS_A_00_TRL_TL_CTL_PD_PREC_SHIFT                      3
#define BCHP_DS_A_00_TRL_TL_CTL_PD_PREC_DEFAULT                    0x00000000

/* DS_A_00_TRL :: TL_CTL :: PD_SEL [02:02] */
#define BCHP_DS_A_00_TRL_TL_CTL_PD_SEL_MASK                        0x00000004
#define BCHP_DS_A_00_TRL_TL_CTL_PD_SEL_SHIFT                       2
#define BCHP_DS_A_00_TRL_TL_CTL_PD_SEL_DEFAULT                     0x00000000

/* DS_A_00_TRL :: TL_CTL :: FRZ [01:01] */
#define BCHP_DS_A_00_TRL_TL_CTL_FRZ_MASK                           0x00000002
#define BCHP_DS_A_00_TRL_TL_CTL_FRZ_SHIFT                          1
#define BCHP_DS_A_00_TRL_TL_CTL_FRZ_DEFAULT                        0x00000001

/* DS_A_00_TRL :: TL_CTL :: RESET [00:00] */
#define BCHP_DS_A_00_TRL_TL_CTL_RESET_MASK                         0x00000001
#define BCHP_DS_A_00_TRL_TL_CTL_RESET_SHIFT                        0
#define BCHP_DS_A_00_TRL_TL_CTL_RESET_DEFAULT                      0x00000001

/***************************************************************************
 *TL_COEFFS - Timing Loop Coefficient Control Register
 ***************************************************************************/
/* DS_A_00_TRL :: TL_COEFFS :: ECO_SPARE_0 [31:21] */
#define BCHP_DS_A_00_TRL_TL_COEFFS_ECO_SPARE_0_MASK                0xffe00000
#define BCHP_DS_A_00_TRL_TL_COEFFS_ECO_SPARE_0_SHIFT               21
#define BCHP_DS_A_00_TRL_TL_COEFFS_ECO_SPARE_0_DEFAULT             0x00000000

/* DS_A_00_TRL :: TL_COEFFS :: KL [20:16] */
#define BCHP_DS_A_00_TRL_TL_COEFFS_KL_MASK                         0x001f0000
#define BCHP_DS_A_00_TRL_TL_COEFFS_KL_SHIFT                        16
#define BCHP_DS_A_00_TRL_TL_COEFFS_KL_DEFAULT                      0x0000001f

/* DS_A_00_TRL :: TL_COEFFS :: ECO_SPARE_1 [15:05] */
#define BCHP_DS_A_00_TRL_TL_COEFFS_ECO_SPARE_1_MASK                0x0000ffe0
#define BCHP_DS_A_00_TRL_TL_COEFFS_ECO_SPARE_1_SHIFT               5
#define BCHP_DS_A_00_TRL_TL_COEFFS_ECO_SPARE_1_DEFAULT             0x00000000

/* DS_A_00_TRL :: TL_COEFFS :: KI [04:00] */
#define BCHP_DS_A_00_TRL_TL_COEFFS_KI_MASK                         0x0000001f
#define BCHP_DS_A_00_TRL_TL_COEFFS_KI_SHIFT                        0
#define BCHP_DS_A_00_TRL_TL_COEFFS_KI_DEFAULT                      0x0000001f

/***************************************************************************
 *TL_FCW - Timing Loop Frequency Control Word Register
 ***************************************************************************/
/* DS_A_00_TRL :: TL_FCW :: FCW [31:08] */
#define BCHP_DS_A_00_TRL_TL_FCW_FCW_MASK                           0xffffff00
#define BCHP_DS_A_00_TRL_TL_FCW_FCW_SHIFT                          8
#define BCHP_DS_A_00_TRL_TL_FCW_FCW_DEFAULT                        0x00800000

/* DS_A_00_TRL :: TL_FCW :: ECO_SPARE_0 [07:00] */
#define BCHP_DS_A_00_TRL_TL_FCW_ECO_SPARE_0_MASK                   0x000000ff
#define BCHP_DS_A_00_TRL_TL_FCW_ECO_SPARE_0_SHIFT                  0
#define BCHP_DS_A_00_TRL_TL_FCW_ECO_SPARE_0_DEFAULT                0x00000000

/***************************************************************************
 *TL_INT - Timing Loop Integrator Register
 ***************************************************************************/
/* DS_A_00_TRL :: TL_INT :: INT [31:00] */
#define BCHP_DS_A_00_TRL_TL_INT_INT_MASK                           0xffffffff
#define BCHP_DS_A_00_TRL_TL_INT_INT_SHIFT                          0
#define BCHP_DS_A_00_TRL_TL_INT_INT_DEFAULT                        0x00000000

/***************************************************************************
 *AGCB_CTL - Digital AGCB Control Register
 ***************************************************************************/
/* DS_A_00_TRL :: AGCB_CTL :: THRESH [31:16] */
#define BCHP_DS_A_00_TRL_AGCB_CTL_THRESH_MASK                      0xffff0000
#define BCHP_DS_A_00_TRL_AGCB_CTL_THRESH_SHIFT                     16
#define BCHP_DS_A_00_TRL_AGCB_CTL_THRESH_DEFAULT                   0x00000000

/* DS_A_00_TRL :: AGCB_CTL :: ECO_SPARE_0 [15:03] */
#define BCHP_DS_A_00_TRL_AGCB_CTL_ECO_SPARE_0_MASK                 0x0000fff8
#define BCHP_DS_A_00_TRL_AGCB_CTL_ECO_SPARE_0_SHIFT                3
#define BCHP_DS_A_00_TRL_AGCB_CTL_ECO_SPARE_0_DEFAULT              0x00000000

/* DS_A_00_TRL :: AGCB_CTL :: DET_SEL [02:02] */
#define BCHP_DS_A_00_TRL_AGCB_CTL_DET_SEL_MASK                     0x00000004
#define BCHP_DS_A_00_TRL_AGCB_CTL_DET_SEL_SHIFT                    2
#define BCHP_DS_A_00_TRL_AGCB_CTL_DET_SEL_DEFAULT                  0x00000000

/* DS_A_00_TRL :: AGCB_CTL :: FRZ [01:01] */
#define BCHP_DS_A_00_TRL_AGCB_CTL_FRZ_MASK                         0x00000002
#define BCHP_DS_A_00_TRL_AGCB_CTL_FRZ_SHIFT                        1
#define BCHP_DS_A_00_TRL_AGCB_CTL_FRZ_DEFAULT                      0x00000001

/* DS_A_00_TRL :: AGCB_CTL :: RESET [00:00] */
#define BCHP_DS_A_00_TRL_AGCB_CTL_RESET_MASK                       0x00000001
#define BCHP_DS_A_00_TRL_AGCB_CTL_RESET_SHIFT                      0
#define BCHP_DS_A_00_TRL_AGCB_CTL_RESET_DEFAULT                    0x00000001

/***************************************************************************
 *AGCB_COEFFS - Digital AGCB Coefficient Register
 ***************************************************************************/
/* DS_A_00_TRL :: AGCB_COEFFS :: ECO_SPARE_0 [31:21] */
#define BCHP_DS_A_00_TRL_AGCB_COEFFS_ECO_SPARE_0_MASK              0xffe00000
#define BCHP_DS_A_00_TRL_AGCB_COEFFS_ECO_SPARE_0_SHIFT             21
#define BCHP_DS_A_00_TRL_AGCB_COEFFS_ECO_SPARE_0_DEFAULT           0x00000000

/* DS_A_00_TRL :: AGCB_COEFFS :: KL [20:16] */
#define BCHP_DS_A_00_TRL_AGCB_COEFFS_KL_MASK                       0x001f0000
#define BCHP_DS_A_00_TRL_AGCB_COEFFS_KL_SHIFT                      16
#define BCHP_DS_A_00_TRL_AGCB_COEFFS_KL_DEFAULT                    0x0000001f

/* DS_A_00_TRL :: AGCB_COEFFS :: ECO_SPARE_1 [15:00] */
#define BCHP_DS_A_00_TRL_AGCB_COEFFS_ECO_SPARE_1_MASK              0x0000ffff
#define BCHP_DS_A_00_TRL_AGCB_COEFFS_ECO_SPARE_1_SHIFT             0
#define BCHP_DS_A_00_TRL_AGCB_COEFFS_ECO_SPARE_1_DEFAULT           0x00000000

/***************************************************************************
 *AGCB_GACC - Digital AGCB Gain Accumulator Register
 ***************************************************************************/
/* DS_A_00_TRL :: AGCB_GACC :: GACC [31:00] */
#define BCHP_DS_A_00_TRL_AGCB_GACC_GACC_MASK                       0xffffffff
#define BCHP_DS_A_00_TRL_AGCB_GACC_GACC_SHIFT                      0
#define BCHP_DS_A_00_TRL_AGCB_GACC_GACC_DEFAULT                    0x04000000

/***************************************************************************
 *US_FCW - UpStream Interface Frequency Control Word
 ***************************************************************************/
/* DS_A_00_TRL :: US_FCW :: FCW [31:00] */
#define BCHP_DS_A_00_TRL_US_FCW_FCW_MASK                           0xffffffff
#define BCHP_DS_A_00_TRL_US_FCW_FCW_SHIFT                          0
#define BCHP_DS_A_00_TRL_US_FCW_FCW_DEFAULT                        0x00008000

/***************************************************************************
 *US_TL_OFFSET - UpStream Interface Timing Offeset
 ***************************************************************************/
/* DS_A_00_TRL :: US_TL_OFFSET :: TL_OFFSET [31:00] */
#define BCHP_DS_A_00_TRL_US_TL_OFFSET_TL_OFFSET_MASK               0xffffffff
#define BCHP_DS_A_00_TRL_US_TL_OFFSET_TL_OFFSET_SHIFT              0
#define BCHP_DS_A_00_TRL_US_TL_OFFSET_TL_OFFSET_DEFAULT            0x00000000

/***************************************************************************
 *HIST_%i - History memory content
 ***************************************************************************/
#define BCHP_DS_A_00_TRL_HIST_i_ARRAY_BASE                         0x02220424
#define BCHP_DS_A_00_TRL_HIST_i_ARRAY_START                        0
#define BCHP_DS_A_00_TRL_HIST_i_ARRAY_END                          182
#define BCHP_DS_A_00_TRL_HIST_i_ARRAY_ELEMENT_SIZE                 32

/***************************************************************************
 *HIST_%i - History memory content
 ***************************************************************************/
/* DS_A_00_TRL :: HIST_i :: DATA [31:00] */
#define BCHP_DS_A_00_TRL_HIST_i_DATA_MASK                          0xffffffff
#define BCHP_DS_A_00_TRL_HIST_i_DATA_SHIFT                         0
#define BCHP_DS_A_00_TRL_HIST_i_DATA_DEFAULT                       0x00000000


#endif /* #ifndef BCHP_DS_A_00_TRL_H__ */

/* End of File */
