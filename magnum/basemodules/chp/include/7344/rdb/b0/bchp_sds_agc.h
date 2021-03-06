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
 * Date:           Generated on         Fri Apr  1 16:47:12 2011
 *                 MD5 Checksum         d03d08c4839c3311c9d35c4cd5e10373
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

#ifndef BCHP_SDS_AGC_H__
#define BCHP_SDS_AGC_H__

/***************************************************************************
 *SDS_AGC - SDS AGC Register Set
 ***************************************************************************/
#define BCHP_SDS_AGC_AGCCTL                      0x01100100 /* AGC Control (Formerly, AGCMISC, AGTCTL, AGICTL) */
#define BCHP_SDS_AGC_IAGCTH                      0x01100104 /* Internal tuner AGC threshold */
#define BCHP_SDS_AGC_DSGIN                       0x01100108 /* DSGIN Register */
#define BCHP_SDS_AGC_ATHR                        0x0110010c /* IF AGC loop threshold */
#define BCHP_SDS_AGC_ABW                         0x01100110 /* IF and RF AGC loop bandwidths */
#define BCHP_SDS_AGC_AII                         0x01100114 /* IF AGC integrator */
#define BCHP_SDS_AGC_AGI                         0x01100118 /* IF AGC gain threshold */
#define BCHP_SDS_AGC_AIT                         0x0110011c /* RF AGC integrator */
#define BCHP_SDS_AGC_AGT                         0x01100120 /* RF AGC gain threshold */
#define BCHP_SDS_AGC_AGCLI                       0x01100124 /* IF AGC delta-sigma fixed gain value and input select. */

/***************************************************************************
 *AGCCTL - AGC Control (Formerly, AGCMISC, AGTCTL, AGICTL)
 ***************************************************************************/
/* SDS_AGC :: AGCCTL :: reserved0 [31:24] */
#define BCHP_SDS_AGC_AGCCTL_reserved0_MASK                         0xff000000
#define BCHP_SDS_AGC_AGCCTL_reserved0_SHIFT                        24

/* SDS_AGC :: AGCCTL :: reserved_for_eco1 [23:23] */
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco1_MASK                 0x00800000
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco1_SHIFT                23
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco1_DEFAULT              0

/* SDS_AGC :: AGCCTL :: SWAPIT [22:22] */
#define BCHP_SDS_AGC_AGCCTL_SWAPIT_MASK                            0x00400000
#define BCHP_SDS_AGC_AGCCTL_SWAPIT_SHIFT                           22
#define BCHP_SDS_AGC_AGCCTL_SWAPIT_DEFAULT                         0

/* SDS_AGC :: AGCCTL :: USEIORQ [21:21] */
#define BCHP_SDS_AGC_AGCCTL_USEIORQ_MASK                           0x00200000
#define BCHP_SDS_AGC_AGCCTL_USEIORQ_SHIFT                          21
#define BCHP_SDS_AGC_AGCCTL_USEIORQ_DEFAULT                        0

/* SDS_AGC :: AGCCTL :: USEQ [20:20] */
#define BCHP_SDS_AGC_AGCCTL_USEQ_MASK                              0x00100000
#define BCHP_SDS_AGC_AGCCTL_USEQ_SHIFT                             20
#define BCHP_SDS_AGC_AGCCTL_USEQ_DEFAULT                           0

/* SDS_AGC :: AGCCTL :: reserved_for_eco2 [19:19] */
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco2_MASK                 0x00080000
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco2_SHIFT                19
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco2_DEFAULT              0

/* SDS_AGC :: AGCCTL :: FRZHLD [18:18] */
#define BCHP_SDS_AGC_AGCCTL_FRZHLD_MASK                            0x00040000
#define BCHP_SDS_AGC_AGCCTL_FRZHLD_SHIFT                           18
#define BCHP_SDS_AGC_AGCCTL_FRZHLD_DEFAULT                         0

/* SDS_AGC :: AGCCTL :: USEST [17:17] */
#define BCHP_SDS_AGC_AGCCTL_USEST_MASK                             0x00020000
#define BCHP_SDS_AGC_AGCCTL_USEST_SHIFT                            17
#define BCHP_SDS_AGC_AGCCTL_USEST_DEFAULT                          0

/* SDS_AGC :: AGCCTL :: STATUS [16:16] */
#define BCHP_SDS_AGC_AGCCTL_STATUS_MASK                            0x00010000
#define BCHP_SDS_AGC_AGCCTL_STATUS_SHIFT                           16
#define BCHP_SDS_AGC_AGCCTL_STATUS_DEFAULT                         0

/* SDS_AGC :: AGCCTL :: reserved_for_eco3 [15:14] */
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco3_MASK                 0x0000c000
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco3_SHIFT                14
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco3_DEFAULT              0

/* SDS_AGC :: AGCCTL :: DSGISELT [13:13] */
#define BCHP_SDS_AGC_AGCCTL_DSGISELT_MASK                          0x00002000
#define BCHP_SDS_AGC_AGCCTL_DSGISELT_SHIFT                         13
#define BCHP_SDS_AGC_AGCCTL_DSGISELT_DEFAULT                       0

/* SDS_AGC :: AGCCTL :: DSGRSTT [12:12] */
#define BCHP_SDS_AGC_AGCCTL_DSGRSTT_MASK                           0x00001000
#define BCHP_SDS_AGC_AGCCTL_DSGRSTT_SHIFT                          12
#define BCHP_SDS_AGC_AGCCTL_DSGRSTT_DEFAULT                        0

/* SDS_AGC :: AGCCTL :: AGTPOS [11:11] */
#define BCHP_SDS_AGC_AGCCTL_AGTPOS_MASK                            0x00000800
#define BCHP_SDS_AGC_AGCCTL_AGTPOS_SHIFT                           11
#define BCHP_SDS_AGC_AGCCTL_AGTPOS_DEFAULT                         1

/* SDS_AGC :: AGCCTL :: AGTZ [10:10] */
#define BCHP_SDS_AGC_AGCCTL_AGTZ_MASK                              0x00000400
#define BCHP_SDS_AGC_AGCCTL_AGTZ_SHIFT                             10
#define BCHP_SDS_AGC_AGCCTL_AGTZ_DEFAULT                           1

/* SDS_AGC :: AGCCTL :: AGTFRZ [09:09] */
#define BCHP_SDS_AGC_AGCCTL_AGTFRZ_MASK                            0x00000200
#define BCHP_SDS_AGC_AGCCTL_AGTFRZ_SHIFT                           9
#define BCHP_SDS_AGC_AGCCTL_AGTFRZ_DEFAULT                         1

/* SDS_AGC :: AGCCTL :: AGTRST [08:08] */
#define BCHP_SDS_AGC_AGCCTL_AGTRST_MASK                            0x00000100
#define BCHP_SDS_AGC_AGCCTL_AGTRST_SHIFT                           8
#define BCHP_SDS_AGC_AGCCTL_AGTRST_DEFAULT                         0

/* SDS_AGC :: AGCCTL :: reserved_for_eco4 [07:06] */
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco4_MASK                 0x000000c0
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco4_SHIFT                6
#define BCHP_SDS_AGC_AGCCTL_reserved_for_eco4_DEFAULT              0

/* SDS_AGC :: AGCCTL :: DSGISELI [05:05] */
#define BCHP_SDS_AGC_AGCCTL_DSGISELI_MASK                          0x00000020
#define BCHP_SDS_AGC_AGCCTL_DSGISELI_SHIFT                         5
#define BCHP_SDS_AGC_AGCCTL_DSGISELI_DEFAULT                       0

/* SDS_AGC :: AGCCTL :: DSGRSTI [04:04] */
#define BCHP_SDS_AGC_AGCCTL_DSGRSTI_MASK                           0x00000010
#define BCHP_SDS_AGC_AGCCTL_DSGRSTI_SHIFT                          4
#define BCHP_SDS_AGC_AGCCTL_DSGRSTI_DEFAULT                        0

/* SDS_AGC :: AGCCTL :: AGIPOS [03:03] */
#define BCHP_SDS_AGC_AGCCTL_AGIPOS_MASK                            0x00000008
#define BCHP_SDS_AGC_AGCCTL_AGIPOS_SHIFT                           3
#define BCHP_SDS_AGC_AGCCTL_AGIPOS_DEFAULT                         1

/* SDS_AGC :: AGCCTL :: AGIZ [02:02] */
#define BCHP_SDS_AGC_AGCCTL_AGIZ_MASK                              0x00000004
#define BCHP_SDS_AGC_AGCCTL_AGIZ_SHIFT                             2
#define BCHP_SDS_AGC_AGCCTL_AGIZ_DEFAULT                           1

/* SDS_AGC :: AGCCTL :: AGIFRZ [01:01] */
#define BCHP_SDS_AGC_AGCCTL_AGIFRZ_MASK                            0x00000002
#define BCHP_SDS_AGC_AGCCTL_AGIFRZ_SHIFT                           1
#define BCHP_SDS_AGC_AGCCTL_AGIFRZ_DEFAULT                         1

/* SDS_AGC :: AGCCTL :: AGIRST [00:00] */
#define BCHP_SDS_AGC_AGCCTL_AGIRST_MASK                            0x00000001
#define BCHP_SDS_AGC_AGCCTL_AGIRST_SHIFT                           0
#define BCHP_SDS_AGC_AGCCTL_AGIRST_DEFAULT                         0

/***************************************************************************
 *IAGCTH - Internal tuner AGC threshold
 ***************************************************************************/
/* SDS_AGC :: IAGCTH :: IAGCTH_I [31:16] */
#define BCHP_SDS_AGC_IAGCTH_IAGCTH_I_MASK                          0xffff0000
#define BCHP_SDS_AGC_IAGCTH_IAGCTH_I_SHIFT                         16
#define BCHP_SDS_AGC_IAGCTH_IAGCTH_I_DEFAULT                       32768

/* SDS_AGC :: IAGCTH :: IAGCTH_T [15:00] */
#define BCHP_SDS_AGC_IAGCTH_IAGCTH_T_MASK                          0x0000ffff
#define BCHP_SDS_AGC_IAGCTH_IAGCTH_T_SHIFT                         0
#define BCHP_SDS_AGC_IAGCTH_IAGCTH_T_DEFAULT                       32768

/***************************************************************************
 *DSGIN - DSGIN Register
 ***************************************************************************/
/* SDS_AGC :: DSGIN :: DSGIN_I [31:16] */
#define BCHP_SDS_AGC_DSGIN_DSGIN_I_MASK                            0xffff0000
#define BCHP_SDS_AGC_DSGIN_DSGIN_I_SHIFT                           16
#define BCHP_SDS_AGC_DSGIN_DSGIN_I_DEFAULT                         0

/* SDS_AGC :: DSGIN :: DSGIN_T [15:00] */
#define BCHP_SDS_AGC_DSGIN_DSGIN_T_MASK                            0x0000ffff
#define BCHP_SDS_AGC_DSGIN_DSGIN_T_SHIFT                           0
#define BCHP_SDS_AGC_DSGIN_DSGIN_T_DEFAULT                         0

/***************************************************************************
 *ATHR - IF AGC loop threshold
 ***************************************************************************/
/* SDS_AGC :: ATHR :: THD [31:16] */
#define BCHP_SDS_AGC_ATHR_THD_MASK                                 0xffff0000
#define BCHP_SDS_AGC_ATHR_THD_SHIFT                                16
#define BCHP_SDS_AGC_ATHR_THD_DEFAULT                              0

/* SDS_AGC :: ATHR :: reserved0 [15:00] */
#define BCHP_SDS_AGC_ATHR_reserved0_MASK                           0x0000ffff
#define BCHP_SDS_AGC_ATHR_reserved0_SHIFT                          0

/***************************************************************************
 *ABW - IF and RF AGC loop bandwidths
 ***************************************************************************/
/* SDS_AGC :: ABW :: reserved0 [31:28] */
#define BCHP_SDS_AGC_ABW_reserved0_MASK                            0xf0000000
#define BCHP_SDS_AGC_ABW_reserved0_SHIFT                           28

/* SDS_AGC :: ABW :: BW_IF [27:24] */
#define BCHP_SDS_AGC_ABW_BW_IF_MASK                                0x0f000000
#define BCHP_SDS_AGC_ABW_BW_IF_SHIFT                               24
#define BCHP_SDS_AGC_ABW_BW_IF_DEFAULT                             3

/* SDS_AGC :: ABW :: reserved1 [23:20] */
#define BCHP_SDS_AGC_ABW_reserved1_MASK                            0x00f00000
#define BCHP_SDS_AGC_ABW_reserved1_SHIFT                           20

/* SDS_AGC :: ABW :: BW_RF [19:16] */
#define BCHP_SDS_AGC_ABW_BW_RF_MASK                                0x000f0000
#define BCHP_SDS_AGC_ABW_BW_RF_SHIFT                               16
#define BCHP_SDS_AGC_ABW_BW_RF_DEFAULT                             3

/* SDS_AGC :: ABW :: reserved2 [15:00] */
#define BCHP_SDS_AGC_ABW_reserved2_MASK                            0x0000ffff
#define BCHP_SDS_AGC_ABW_reserved2_SHIFT                           0

/***************************************************************************
 *AII - IF AGC integrator
 ***************************************************************************/
/* SDS_AGC :: AII :: INT_IF [31:08] */
#define BCHP_SDS_AGC_AII_INT_IF_MASK                               0xffffff00
#define BCHP_SDS_AGC_AII_INT_IF_SHIFT                              8
#define BCHP_SDS_AGC_AII_INT_IF_DEFAULT                            8388608

/* SDS_AGC :: AII :: reserved0 [07:00] */
#define BCHP_SDS_AGC_AII_reserved0_MASK                            0x000000ff
#define BCHP_SDS_AGC_AII_reserved0_SHIFT                           0

/***************************************************************************
 *AGI - IF AGC gain threshold
 ***************************************************************************/
/* SDS_AGC :: AGI :: GAIN_IF [31:04] */
#define BCHP_SDS_AGC_AGI_GAIN_IF_MASK                              0xfffffff0
#define BCHP_SDS_AGC_AGI_GAIN_IF_SHIFT                             4
#define BCHP_SDS_AGC_AGI_GAIN_IF_DEFAULT                           0

/* SDS_AGC :: AGI :: reserved0 [03:01] */
#define BCHP_SDS_AGC_AGI_reserved0_MASK                            0x0000000e
#define BCHP_SDS_AGC_AGI_reserved0_SHIFT                           1

/* SDS_AGC :: AGI :: GAIN_IF_STS [00:00] */
#define BCHP_SDS_AGC_AGI_GAIN_IF_STS_MASK                          0x00000001
#define BCHP_SDS_AGC_AGI_GAIN_IF_STS_SHIFT                         0
#define BCHP_SDS_AGC_AGI_GAIN_IF_STS_DEFAULT                       0

/***************************************************************************
 *AIT - RF AGC integrator
 ***************************************************************************/
/* SDS_AGC :: AIT :: INT_RF [31:08] */
#define BCHP_SDS_AGC_AIT_INT_RF_MASK                               0xffffff00
#define BCHP_SDS_AGC_AIT_INT_RF_SHIFT                              8
#define BCHP_SDS_AGC_AIT_INT_RF_DEFAULT                            8388607

/* SDS_AGC :: AIT :: reserved0 [07:00] */
#define BCHP_SDS_AGC_AIT_reserved0_MASK                            0x000000ff
#define BCHP_SDS_AGC_AIT_reserved0_SHIFT                           0

/***************************************************************************
 *AGT - RF AGC gain threshold
 ***************************************************************************/
/* SDS_AGC :: AGT :: GAIN_RF [31:04] */
#define BCHP_SDS_AGC_AGT_GAIN_RF_MASK                              0xfffffff0
#define BCHP_SDS_AGC_AGT_GAIN_RF_SHIFT                             4
#define BCHP_SDS_AGC_AGT_GAIN_RF_DEFAULT                           268435455

/* SDS_AGC :: AGT :: reserved0 [03:01] */
#define BCHP_SDS_AGC_AGT_reserved0_MASK                            0x0000000e
#define BCHP_SDS_AGC_AGT_reserved0_SHIFT                           1

/* SDS_AGC :: AGT :: GAIN_IF_STS [00:00] */
#define BCHP_SDS_AGC_AGT_GAIN_IF_STS_MASK                          0x00000001
#define BCHP_SDS_AGC_AGT_GAIN_IF_STS_SHIFT                         0
#define BCHP_SDS_AGC_AGT_GAIN_IF_STS_DEFAULT                       0

/***************************************************************************
 *AGCLI - IF AGC delta-sigma fixed gain value and input select.
 ***************************************************************************/
/* SDS_AGC :: AGCLI :: XIF_SQ [31:12] */
#define BCHP_SDS_AGC_AGCLI_XIF_SQ_MASK                             0xfffff000
#define BCHP_SDS_AGC_AGCLI_XIF_SQ_SHIFT                            12
#define BCHP_SDS_AGC_AGCLI_XIF_SQ_DEFAULT                          0

/* SDS_AGC :: AGCLI :: reserved0 [11:00] */
#define BCHP_SDS_AGC_AGCLI_reserved0_MASK                          0x00000fff
#define BCHP_SDS_AGC_AGCLI_reserved0_SHIFT                         0

#endif /* #ifndef BCHP_SDS_AGC_H__ */

/* End of File */
