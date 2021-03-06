/***************************************************************************
 *     Copyright (c) 1999-2013, Broadcom Corporation
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
 * Date:           Generated on              Fri Oct 18 11:45:08 2013
 *                 Full Compile MD5 Checksum 32c6546d3ec1c83fbf8bc6d4e3ec46d1
 *                   (minus title and desc)  
 *                 MD5 Checksum              839f407f1c04ba70b32cb458815eb9b4
 *
 * Compiled with:  RDB Utility               combo_header.pl
 *                 RDB Parser                3.0
 *                 unknown                   unknown
 *                 Perl Interpreter          5.008008
 *                 Operating System          linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_SDS_BERT_0_H__
#define BCHP_SDS_BERT_0_H__

/***************************************************************************
 *SDS_BERT_0 - SDS BERT Register Set
 ***************************************************************************/
#define BCHP_SDS_BERT_0_BERCTL                   0x000c0540 /* BERT Control Register (Formerly,BERCTLA,BERCTLB) */
#define BCHP_SDS_BERT_0_BEIT                     0x000c0544 /* BERT integration period and threshold */
#define BCHP_SDS_BERT_0_BERC                     0x000c0548 /* BERT error counter value */
#define BCHP_SDS_BERT_0_BEM1                     0x000c054c /* BERT 16-QAM or 8-PSK symbol mapping values */
#define BCHP_SDS_BERT_0_BEM2                     0x000c0550 /* BERT 16-QAM symbol mapping values */
#define BCHP_SDS_BERT_0_BEM3                     0x000c0554 /* BERT QPSK symbol mapping values */
#define BCHP_SDS_BERT_0_BEST                     0x000c0558 /* BERT status register */
#define BCHP_SDS_BERT_0_ACMCTL                   0x000c055c /* BERT ACM Control register */

/***************************************************************************
 *BERCTL - BERT Control Register (Formerly,BERCTLA,BERCTLB)
 ***************************************************************************/
/* SDS_BERT_0 :: BERCTL :: reserved0 [31:17] */
#define BCHP_SDS_BERT_0_BERCTL_reserved0_MASK                      0xfffe0000
#define BCHP_SDS_BERT_0_BERCTL_reserved0_SHIFT                     17

/* SDS_BERT_0 :: BERCTL :: BERSEQ2 [16:16] */
#define BCHP_SDS_BERT_0_BERCTL_BERSEQ2_MASK                        0x00010000
#define BCHP_SDS_BERT_0_BERCTL_BERSEQ2_SHIFT                       16
#define BCHP_SDS_BERT_0_BERCTL_BERSEQ2_DEFAULT                     0x00000000

/* SDS_BERT_0 :: BERCTL :: BERIQR [15:12] */
#define BCHP_SDS_BERT_0_BERCTL_BERIQR_MASK                         0x0000f000
#define BCHP_SDS_BERT_0_BERCTL_BERIQR_SHIFT                        12
#define BCHP_SDS_BERT_0_BERCTL_BERIQR_DEFAULT                      0x00000000

/* SDS_BERT_0 :: BERCTL :: BERRMD [11:11] */
#define BCHP_SDS_BERT_0_BERCTL_BERRMD_MASK                         0x00000800
#define BCHP_SDS_BERT_0_BERCTL_BERRMD_SHIFT                        11
#define BCHP_SDS_BERT_0_BERCTL_BERRMD_DEFAULT                      0x00000000

/* SDS_BERT_0 :: BERCTL :: BERMOD [10:10] */
#define BCHP_SDS_BERT_0_BERCTL_BERMOD_MASK                         0x00000400
#define BCHP_SDS_BERT_0_BERCTL_BERMOD_SHIFT                        10
#define BCHP_SDS_BERT_0_BERCTL_BERMOD_DEFAULT                      0x00000000

/* SDS_BERT_0 :: BERCTL :: BERINV [09:09] */
#define BCHP_SDS_BERT_0_BERCTL_BERINV_MASK                         0x00000200
#define BCHP_SDS_BERT_0_BERCTL_BERINV_SHIFT                        9
#define BCHP_SDS_BERT_0_BERCTL_BERINV_DEFAULT                      0x00000000

/* SDS_BERT_0 :: BERCTL :: BERSEQ [08:08] */
#define BCHP_SDS_BERT_0_BERCTL_BERSEQ_MASK                         0x00000100
#define BCHP_SDS_BERT_0_BERCTL_BERSEQ_SHIFT                        8
#define BCHP_SDS_BERT_0_BERCTL_BERSEQ_DEFAULT                      0x00000000

/* SDS_BERT_0 :: BERCTL :: BERSRC [07:04] */
#define BCHP_SDS_BERT_0_BERCTL_BERSRC_MASK                         0x000000f0
#define BCHP_SDS_BERT_0_BERCTL_BERSRC_SHIFT                        4
#define BCHP_SDS_BERT_0_BERCTL_BERSRC_DEFAULT                      0x0000000a

/* SDS_BERT_0 :: BERCTL :: BERPSV [03:03] */
#define BCHP_SDS_BERT_0_BERCTL_BERPSV_MASK                         0x00000008
#define BCHP_SDS_BERT_0_BERCTL_BERPSV_SHIFT                        3
#define BCHP_SDS_BERT_0_BERCTL_BERPSV_DEFAULT                      0x00000000

/* SDS_BERT_0 :: BERCTL :: BEROON [02:02] */
#define BCHP_SDS_BERT_0_BERCTL_BEROON_MASK                         0x00000004
#define BCHP_SDS_BERT_0_BERCTL_BEROON_SHIFT                        2
#define BCHP_SDS_BERT_0_BERCTL_BEROON_DEFAULT                      0x00000001

/* SDS_BERT_0 :: BERCTL :: BERCEN [01:01] */
#define BCHP_SDS_BERT_0_BERCTL_BERCEN_MASK                         0x00000002
#define BCHP_SDS_BERT_0_BERCTL_BERCEN_SHIFT                        1
#define BCHP_SDS_BERT_0_BERCTL_BERCEN_DEFAULT                      0x00000000

/* SDS_BERT_0 :: BERCTL :: BERRST [00:00] */
#define BCHP_SDS_BERT_0_BERCTL_BERRST_MASK                         0x00000001
#define BCHP_SDS_BERT_0_BERCTL_BERRST_SHIFT                        0
#define BCHP_SDS_BERT_0_BERCTL_BERRST_DEFAULT                      0x00000000

/***************************************************************************
 *BEIT - BERT integration period and threshold
 ***************************************************************************/
/* SDS_BERT_0 :: BEIT :: iperiod [31:16] */
#define BCHP_SDS_BERT_0_BEIT_iperiod_MASK                          0xffff0000
#define BCHP_SDS_BERT_0_BEIT_iperiod_SHIFT                         16
#define BCHP_SDS_BERT_0_BEIT_iperiod_DEFAULT                       0x000000ff

/* SDS_BERT_0 :: BEIT :: threshold [15:00] */
#define BCHP_SDS_BERT_0_BEIT_threshold_MASK                        0x0000ffff
#define BCHP_SDS_BERT_0_BEIT_threshold_SHIFT                       0
#define BCHP_SDS_BERT_0_BEIT_threshold_DEFAULT                     0x0000003f

/***************************************************************************
 *BERC - BERT error counter value
 ***************************************************************************/
/* SDS_BERT_0 :: BERC :: err_cnt [31:00] */
#define BCHP_SDS_BERT_0_BERC_err_cnt_MASK                          0xffffffff
#define BCHP_SDS_BERT_0_BERC_err_cnt_SHIFT                         0
#define BCHP_SDS_BERT_0_BERC_err_cnt_DEFAULT                       0x00000000

/***************************************************************************
 *BEM1 - BERT 16-QAM or 8-PSK symbol mapping values
 ***************************************************************************/
/* SDS_BERT_0 :: BEM1 :: ae [31:28] */
#define BCHP_SDS_BERT_0_BEM1_ae_MASK                               0xf0000000
#define BCHP_SDS_BERT_0_BEM1_ae_SHIFT                              28
#define BCHP_SDS_BERT_0_BEM1_ae_DEFAULT                            0x00000000

/* SDS_BERT_0 :: BEM1 :: af [27:24] */
#define BCHP_SDS_BERT_0_BEM1_af_MASK                               0x0f000000
#define BCHP_SDS_BERT_0_BEM1_af_SHIFT                              24
#define BCHP_SDS_BERT_0_BEM1_af_DEFAULT                            0x00000000

/* SDS_BERT_0 :: BEM1 :: be [23:20] */
#define BCHP_SDS_BERT_0_BEM1_be_MASK                               0x00f00000
#define BCHP_SDS_BERT_0_BEM1_be_SHIFT                              20
#define BCHP_SDS_BERT_0_BEM1_be_DEFAULT                            0x00000000

/* SDS_BERT_0 :: BEM1 :: bf [19:16] */
#define BCHP_SDS_BERT_0_BEM1_bf_MASK                               0x000f0000
#define BCHP_SDS_BERT_0_BEM1_bf_SHIFT                              16
#define BCHP_SDS_BERT_0_BEM1_bf_DEFAULT                            0x00000006

/* SDS_BERT_0 :: BEM1 :: ce [15:12] */
#define BCHP_SDS_BERT_0_BEM1_ce_MASK                               0x0000f000
#define BCHP_SDS_BERT_0_BEM1_ce_SHIFT                              12
#define BCHP_SDS_BERT_0_BEM1_ce_DEFAULT                            0x0000000c

/* SDS_BERT_0 :: BEM1 :: cf [11:08] */
#define BCHP_SDS_BERT_0_BEM1_cf_MASK                               0x00000f00
#define BCHP_SDS_BERT_0_BEM1_cf_SHIFT                              8
#define BCHP_SDS_BERT_0_BEM1_cf_DEFAULT                            0x0000000d

/* SDS_BERT_0 :: BEM1 :: de [07:04] */
#define BCHP_SDS_BERT_0_BEM1_de_MASK                               0x000000f0
#define BCHP_SDS_BERT_0_BEM1_de_SHIFT                              4
#define BCHP_SDS_BERT_0_BEM1_de_DEFAULT                            0x0000000d

/* SDS_BERT_0 :: BEM1 :: df [03:00] */
#define BCHP_SDS_BERT_0_BEM1_df_MASK                               0x0000000f
#define BCHP_SDS_BERT_0_BEM1_df_SHIFT                              0
#define BCHP_SDS_BERT_0_BEM1_df_DEFAULT                            0x0000000a

/***************************************************************************
 *BEM2 - BERT 16-QAM symbol mapping values
 ***************************************************************************/
/* SDS_BERT_0 :: BEM2 :: ag [31:28] */
#define BCHP_SDS_BERT_0_BEM2_ag_MASK                               0xf0000000
#define BCHP_SDS_BERT_0_BEM2_ag_SHIFT                              28
#define BCHP_SDS_BERT_0_BEM2_ag_DEFAULT                            0x00000000

/* SDS_BERT_0 :: BEM2 :: ah [27:24] */
#define BCHP_SDS_BERT_0_BEM2_ah_MASK                               0x0f000000
#define BCHP_SDS_BERT_0_BEM2_ah_SHIFT                              24
#define BCHP_SDS_BERT_0_BEM2_ah_DEFAULT                            0x00000001

/* SDS_BERT_0 :: BEM2 :: bg [23:20] */
#define BCHP_SDS_BERT_0_BEM2_bg_MASK                               0x00f00000
#define BCHP_SDS_BERT_0_BEM2_bg_SHIFT                              20
#define BCHP_SDS_BERT_0_BEM2_bg_DEFAULT                            0x00000002

/* SDS_BERT_0 :: BEM2 :: bh [19:16] */
#define BCHP_SDS_BERT_0_BEM2_bh_MASK                               0x000f0000
#define BCHP_SDS_BERT_0_BEM2_bh_SHIFT                              16
#define BCHP_SDS_BERT_0_BEM2_bh_DEFAULT                            0x00000003

/* SDS_BERT_0 :: BEM2 :: cg [15:12] */
#define BCHP_SDS_BERT_0_BEM2_cg_MASK                               0x0000f000
#define BCHP_SDS_BERT_0_BEM2_cg_SHIFT                              12
#define BCHP_SDS_BERT_0_BEM2_cg_DEFAULT                            0x00000004

/* SDS_BERT_0 :: BEM2 :: ch [11:08] */
#define BCHP_SDS_BERT_0_BEM2_ch_MASK                               0x00000f00
#define BCHP_SDS_BERT_0_BEM2_ch_SHIFT                              8
#define BCHP_SDS_BERT_0_BEM2_ch_DEFAULT                            0x00000005

/* SDS_BERT_0 :: BEM2 :: dg [07:04] */
#define BCHP_SDS_BERT_0_BEM2_dg_MASK                               0x000000f0
#define BCHP_SDS_BERT_0_BEM2_dg_SHIFT                              4
#define BCHP_SDS_BERT_0_BEM2_dg_DEFAULT                            0x00000006

/* SDS_BERT_0 :: BEM2 :: dh [03:00] */
#define BCHP_SDS_BERT_0_BEM2_dh_MASK                               0x0000000f
#define BCHP_SDS_BERT_0_BEM2_dh_SHIFT                              0
#define BCHP_SDS_BERT_0_BEM2_dh_DEFAULT                            0x00000007

/***************************************************************************
 *BEM3 - BERT QPSK symbol mapping values
 ***************************************************************************/
/* SDS_BERT_0 :: BEM3 :: q_1 [31:30] */
#define BCHP_SDS_BERT_0_BEM3_q_1_MASK                              0xc0000000
#define BCHP_SDS_BERT_0_BEM3_q_1_SHIFT                             30
#define BCHP_SDS_BERT_0_BEM3_q_1_DEFAULT                           0x00000000

/* SDS_BERT_0 :: BEM3 :: q_2 [29:28] */
#define BCHP_SDS_BERT_0_BEM3_q_2_MASK                              0x30000000
#define BCHP_SDS_BERT_0_BEM3_q_2_SHIFT                             28
#define BCHP_SDS_BERT_0_BEM3_q_2_DEFAULT                           0x00000002

/* SDS_BERT_0 :: BEM3 :: q_3 [27:26] */
#define BCHP_SDS_BERT_0_BEM3_q_3_MASK                              0x0c000000
#define BCHP_SDS_BERT_0_BEM3_q_3_SHIFT                             26
#define BCHP_SDS_BERT_0_BEM3_q_3_DEFAULT                           0x00000003

/* SDS_BERT_0 :: BEM3 :: q_4 [25:24] */
#define BCHP_SDS_BERT_0_BEM3_q_4_MASK                              0x03000000
#define BCHP_SDS_BERT_0_BEM3_q_4_SHIFT                             24
#define BCHP_SDS_BERT_0_BEM3_q_4_DEFAULT                           0x00000001

/* SDS_BERT_0 :: BEM3 :: reserved0 [23:00] */
#define BCHP_SDS_BERT_0_BEM3_reserved0_MASK                        0x00ffffff
#define BCHP_SDS_BERT_0_BEM3_reserved0_SHIFT                       0

/***************************************************************************
 *BEST - BERT status register
 ***************************************************************************/
/* SDS_BERT_0 :: BEST :: berrot [31:28] */
#define BCHP_SDS_BERT_0_BEST_berrot_MASK                           0xf0000000
#define BCHP_SDS_BERT_0_BEST_berrot_SHIFT                          28
#define BCHP_SDS_BERT_0_BEST_berrot_DEFAULT                        0x00000000

/* SDS_BERT_0 :: BEST :: sync_lock [27:27] */
#define BCHP_SDS_BERT_0_BEST_sync_lock_MASK                        0x08000000
#define BCHP_SDS_BERT_0_BEST_sync_lock_SHIFT                       27
#define BCHP_SDS_BERT_0_BEST_sync_lock_DEFAULT                     0x00000000

/* SDS_BERT_0 :: BEST :: sync_unlock [26:26] */
#define BCHP_SDS_BERT_0_BEST_sync_unlock_MASK                      0x04000000
#define BCHP_SDS_BERT_0_BEST_sync_unlock_SHIFT                     26
#define BCHP_SDS_BERT_0_BEST_sync_unlock_DEFAULT                   0x00000000

/* SDS_BERT_0 :: BEST :: sync [25:25] */
#define BCHP_SDS_BERT_0_BEST_sync_MASK                             0x02000000
#define BCHP_SDS_BERT_0_BEST_sync_SHIFT                            25
#define BCHP_SDS_BERT_0_BEST_sync_DEFAULT                          0x00000000

/* SDS_BERT_0 :: BEST :: vitsel [24:24] */
#define BCHP_SDS_BERT_0_BEST_vitsel_MASK                           0x01000000
#define BCHP_SDS_BERT_0_BEST_vitsel_SHIFT                          24
#define BCHP_SDS_BERT_0_BEST_vitsel_DEFAULT                        0x00000001

/* SDS_BERT_0 :: BEST :: reserved0 [23:00] */
#define BCHP_SDS_BERT_0_BEST_reserved0_MASK                        0x00ffffff
#define BCHP_SDS_BERT_0_BEST_reserved0_SHIFT                       0

/***************************************************************************
 *ACMCTL - BERT ACM Control register
 ***************************************************************************/
/* SDS_BERT_0 :: ACMCTL :: reserved0 [31:08] */
#define BCHP_SDS_BERT_0_ACMCTL_reserved0_MASK                      0xffffff00
#define BCHP_SDS_BERT_0_ACMCTL_reserved0_SHIFT                     8

/* SDS_BERT_0 :: ACMCTL :: reserved_for_eco1 [07:06] */
#define BCHP_SDS_BERT_0_ACMCTL_reserved_for_eco1_MASK              0x000000c0
#define BCHP_SDS_BERT_0_ACMCTL_reserved_for_eco1_SHIFT             6
#define BCHP_SDS_BERT_0_ACMCTL_reserved_for_eco1_DEFAULT           0x00000000

/* SDS_BERT_0 :: ACMCTL :: acm_bert_en [05:05] */
#define BCHP_SDS_BERT_0_ACMCTL_acm_bert_en_MASK                    0x00000020
#define BCHP_SDS_BERT_0_ACMCTL_acm_bert_en_SHIFT                   5
#define BCHP_SDS_BERT_0_ACMCTL_acm_bert_en_DEFAULT                 0x00000000

/* SDS_BERT_0 :: ACMCTL :: target_modcod [04:00] */
#define BCHP_SDS_BERT_0_ACMCTL_target_modcod_MASK                  0x0000001f
#define BCHP_SDS_BERT_0_ACMCTL_target_modcod_SHIFT                 0
#define BCHP_SDS_BERT_0_ACMCTL_target_modcod_DEFAULT               0x0000000d

#endif /* #ifndef BCHP_SDS_BERT_0_H__ */

/* End of File */
