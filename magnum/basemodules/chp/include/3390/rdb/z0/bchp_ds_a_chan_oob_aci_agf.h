/********************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Thu Mar 31 15:04:54 2016
 *                 Full Compile MD5 Checksum  c4047ee397223298a92cb5ed9f7003ae
 *                     (minus title and desc)
 *                 MD5 Checksum               3a2da73040e5919d5073d624063d2523
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     899
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_DS_A_CHAN_OOB_ACI_AGF_H__
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_H__

/***************************************************************************
 *DS_A_CHAN_OOB_ACI_AGF - Channel Isolation Processor Configuration Registers for OOB
 ***************************************************************************/
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI           0x02210600 /* [RW] CIP ACI control register */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF           0x02210604 /* [RW] Digital AGC(Fine) Control Register */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFI          0x02210608 /* [RW] Digital AGC(Fine) Integrator Value */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFLI         0x0221060c /* [RW] Digital AGC(Fine) Leaky Integrator Value */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AVG       0x02210610 /* [RW] Digital AGC(Fine) Averager Control Register */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFI_AVG      0x02210614 /* [RW] Digital AGC(Fine) Integrator Averager Value */

/***************************************************************************
 *ACI - CIP ACI control register
 ***************************************************************************/
/* DS_A_CHAN_OOB_ACI_AGF :: ACI :: ECO_SPARE_1 [31:13] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ECO_SPARE_1_MASK            0xffffe000
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ECO_SPARE_1_SHIFT           13
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ECO_SPARE_1_DEFAULT         0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: ACI :: FILTC_ODD_LEN [12:12] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_FILTC_ODD_LEN_MASK          0x00001000
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_FILTC_ODD_LEN_SHIFT         12
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_FILTC_ODD_LEN_DEFAULT       0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: ACI :: ACI_BYP [11:11] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ACI_BYP_MASK                0x00000800
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ACI_BYP_SHIFT               11
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ACI_BYP_DEFAULT             0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: ACI :: ACI_COEF_SEL [10:08] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ACI_COEF_SEL_MASK           0x00000700
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ACI_COEF_SEL_SHIFT          8
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ACI_COEF_SEL_DEFAULT        0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: ACI :: ECO_SPARE_0 [07:00] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ECO_SPARE_0_MASK            0x000000ff
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ECO_SPARE_0_SHIFT           0
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_ACI_ECO_SPARE_0_DEFAULT         0x00000000

/***************************************************************************
 *AGF - Digital AGC(Fine) Control Register
 ***************************************************************************/
/* DS_A_CHAN_OOB_ACI_AGF :: AGF :: KL [31:27] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_KL_MASK                     0xf8000000
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_KL_SHIFT                    27
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_KL_DEFAULT                  0x00000009

/* DS_A_CHAN_OOB_ACI_AGF :: AGF :: AGFTHR [26:08] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFTHR_MASK                 0x07ffff00
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFTHR_SHIFT                8
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFTHR_DEFAULT              0x00010000

/* DS_A_CHAN_OOB_ACI_AGF :: AGF :: ECO_SPARE_0 [07:07] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_ECO_SPARE_0_MASK            0x00000080
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_ECO_SPARE_0_SHIFT           7
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_ECO_SPARE_0_DEFAULT         0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: AGF :: LOG2_MODE [06:06] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_LOG2_MODE_MASK              0x00000040
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_LOG2_MODE_SHIFT             6
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_LOG2_MODE_DEFAULT           0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: AGF :: USE_STATUS [05:05] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_USE_STATUS_MASK             0x00000020
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_USE_STATUS_SHIFT            5
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_USE_STATUS_DEFAULT          0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: AGF :: STATUS [04:04] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_STATUS_MASK                 0x00000010
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_STATUS_SHIFT                4
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_STATUS_DEFAULT              0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: AGF :: LKBYP [03:03] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_LKBYP_MASK                  0x00000008
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_LKBYP_SHIFT                 3
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_LKBYP_DEFAULT               0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: AGF :: AGFFRZ [02:02] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFFRZ_MASK                 0x00000004
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFFRZ_SHIFT                2
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFFRZ_DEFAULT              0x00000001

/* DS_A_CHAN_OOB_ACI_AGF :: AGF :: AGFBYP [01:01] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFBYP_MASK                 0x00000002
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFBYP_SHIFT                1
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFBYP_DEFAULT              0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: AGF :: AGFRST [00:00] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFRST_MASK                 0x00000001
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFRST_SHIFT                0
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AGFRST_DEFAULT              0x00000000

/***************************************************************************
 *AGFI - Digital AGC(Fine) Integrator Value
 ***************************************************************************/
/* DS_A_CHAN_OOB_ACI_AGF :: AGFI :: AGFVAL [31:00] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFI_AGFVAL_MASK                0xffffffff
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFI_AGFVAL_SHIFT               0
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFI_AGFVAL_DEFAULT             0x00010000

/***************************************************************************
 *AGFLI - Digital AGC(Fine) Leaky Integrator Value
 ***************************************************************************/
/* DS_A_CHAN_OOB_ACI_AGF :: AGFLI :: ECO_SPARE_0 [31:23] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFLI_ECO_SPARE_0_MASK          0xff800000
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFLI_ECO_SPARE_0_SHIFT         23
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFLI_ECO_SPARE_0_DEFAULT       0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: AGFLI :: AGFLVAL [22:00] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFLI_AGFLVAL_MASK              0x007fffff
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFLI_AGFLVAL_SHIFT             0
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFLI_AGFLVAL_DEFAULT           0x00000000

/***************************************************************************
 *AGF_AVG - Digital AGC(Fine) Averager Control Register
 ***************************************************************************/
/* DS_A_CHAN_OOB_ACI_AGF :: AGF_AVG :: KL_AVG [31:27] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AVG_KL_AVG_MASK             0xf8000000
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AVG_KL_AVG_SHIFT            27
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AVG_KL_AVG_DEFAULT          0x00000000

/* DS_A_CHAN_OOB_ACI_AGF :: AGF_AVG :: ECO_SPARE_0 [26:00] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AVG_ECO_SPARE_0_MASK        0x07ffffff
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AVG_ECO_SPARE_0_SHIFT       0
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGF_AVG_ECO_SPARE_0_DEFAULT     0x00000000

/***************************************************************************
 *AGFI_AVG - Digital AGC(Fine) Integrator Averager Value
 ***************************************************************************/
/* DS_A_CHAN_OOB_ACI_AGF :: AGFI_AVG :: AGFVAL [31:00] */
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFI_AVG_AGFVAL_MASK            0xffffffff
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFI_AVG_AGFVAL_SHIFT           0
#define BCHP_DS_A_CHAN_OOB_ACI_AGF_AGFI_AVG_AGFVAL_DEFAULT         0x00000000

#endif /* #ifndef BCHP_DS_A_CHAN_OOB_ACI_AGF_H__ */

/* End of File */
