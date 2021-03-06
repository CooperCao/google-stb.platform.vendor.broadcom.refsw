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
 * Date:           Generated on         Fri Jan 22 20:05:53 2010
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

#ifndef BCHP_DS_TUNER_CHNZ_0_H__
#define BCHP_DS_TUNER_CHNZ_0_H__

/***************************************************************************
 *DS_TUNER_CHNZ_0 - Channelizer Registers 0
 ***************************************************************************/
#define BCHP_DS_TUNER_CHNZ_0_FE                  0x04c3c000 /* Digital Front End Control Register */
#define BCHP_DS_TUNER_CHNZ_0_SGFE                0x04c3c004 /* Front end Signature Analyzer */
#define BCHP_DS_TUNER_CHNZ_0_ADC                 0x04c3c008 /* Current ADC Output Sample */
#define BCHP_DS_TUNER_CHNZ_0_AGF                 0x04c3c00c /* Digital AGC(Fine) Control Register */
#define BCHP_DS_TUNER_CHNZ_0_AGFI                0x04c3c010 /* Digital AGC(Fine) Integrator Value */
#define BCHP_DS_TUNER_CHNZ_0_AGFLI               0x04c3c014 /* Digital AGC(Fine) Leaky Integrator Value */
#define BCHP_DS_TUNER_CHNZ_0_CFLOS               0x04c3c018 /* Carrier Frequency Loop Cosine/Sine */
#define BCHP_DS_TUNER_CHNZ_0_DPM                 0x04c3c01c /* DPM register for control signals */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCI             0x04c3c020 /* DPM DC Canceller I Integrator Value */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCQ             0x04c3c024 /* DPM DC Canceller Q Integrator Value */
#define BCHP_DS_TUNER_CHNZ_0_DPMK2               0x04c3c028 /* DPM register for control signals */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCK2I           0x04c3c02c /* DPM DC Canceller I Integrator Value */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCK2Q           0x04c3c030 /* DPM DC Canceller Q Integrator Value */

/***************************************************************************
 *FE - Digital Front End Control Register
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: FE :: SKIP_1CLK [31:31] */
#define BCHP_DS_TUNER_CHNZ_0_FE_SKIP_1CLK_MASK                     0x80000000
#define BCHP_DS_TUNER_CHNZ_0_FE_SKIP_1CLK_SHIFT                    31

/* DS_TUNER_CHNZ_0 :: FE :: TPOUT_SEL [30:26] */
#define BCHP_DS_TUNER_CHNZ_0_FE_TPOUT_SEL_MASK                     0x7c000000
#define BCHP_DS_TUNER_CHNZ_0_FE_TPOUT_SEL_SHIFT                    26

/* DS_TUNER_CHNZ_0 :: FE :: MIXMODE_IR [25:24] */
#define BCHP_DS_TUNER_CHNZ_0_FE_MIXMODE_IR_MASK                    0x03000000
#define BCHP_DS_TUNER_CHNZ_0_FE_MIXMODE_IR_SHIFT                   24

/* DS_TUNER_CHNZ_0 :: FE :: skip_1clock_ir_IR [23:23] */
#define BCHP_DS_TUNER_CHNZ_0_FE_skip_1clock_ir_IR_MASK             0x00800000
#define BCHP_DS_TUNER_CHNZ_0_FE_skip_1clock_ir_IR_SHIFT            23

/* DS_TUNER_CHNZ_0 :: FE :: skip_1clock_ir [22:22] */
#define BCHP_DS_TUNER_CHNZ_0_FE_skip_1clock_ir_MASK                0x00400000
#define BCHP_DS_TUNER_CHNZ_0_FE_skip_1clock_ir_SHIFT               22

/* DS_TUNER_CHNZ_0 :: FE :: bypmix_ir [21:21] */
#define BCHP_DS_TUNER_CHNZ_0_FE_bypmix_ir_MASK                     0x00200000
#define BCHP_DS_TUNER_CHNZ_0_FE_bypmix_ir_SHIFT                    21

/* DS_TUNER_CHNZ_0 :: FE :: inpsel_ir [20:20] */
#define BCHP_DS_TUNER_CHNZ_0_FE_inpsel_ir_MASK                     0x00100000
#define BCHP_DS_TUNER_CHNZ_0_FE_inpsel_ir_SHIFT                    20

/* DS_TUNER_CHNZ_0 :: FE :: IR_EN [19:19] */
#define BCHP_DS_TUNER_CHNZ_0_FE_IR_EN_MASK                         0x00080000
#define BCHP_DS_TUNER_CHNZ_0_FE_IR_EN_SHIFT                        19

/* DS_TUNER_CHNZ_0 :: FE :: SKIP_1CLK_Q [18:18] */
#define BCHP_DS_TUNER_CHNZ_0_FE_SKIP_1CLK_Q_MASK                   0x00040000
#define BCHP_DS_TUNER_CHNZ_0_FE_SKIP_1CLK_Q_SHIFT                  18

/* DS_TUNER_CHNZ_0 :: FE :: MIXMODE [17:16] */
#define BCHP_DS_TUNER_CHNZ_0_FE_MIXMODE_MASK                       0x00030000
#define BCHP_DS_TUNER_CHNZ_0_FE_MIXMODE_SHIFT                      16

/* DS_TUNER_CHNZ_0 :: FE :: FILTC_ODD_LENGTH [15:15] */
#define BCHP_DS_TUNER_CHNZ_0_FE_FILTC_ODD_LENGTH_MASK              0x00008000
#define BCHP_DS_TUNER_CHNZ_0_FE_FILTC_ODD_LENGTH_SHIFT             15

/* DS_TUNER_CHNZ_0 :: FE :: FILTC_TPIN_SEL [14:14] */
#define BCHP_DS_TUNER_CHNZ_0_FE_FILTC_TPIN_SEL_MASK                0x00004000
#define BCHP_DS_TUNER_CHNZ_0_FE_FILTC_TPIN_SEL_SHIFT               14

/* DS_TUNER_CHNZ_0 :: FE :: EN_INTP [13:13] */
#define BCHP_DS_TUNER_CHNZ_0_FE_EN_INTP_MASK                       0x00002000
#define BCHP_DS_TUNER_CHNZ_0_FE_EN_INTP_SHIFT                      13

/* DS_TUNER_CHNZ_0 :: FE :: BYPHA [12:12] */
#define BCHP_DS_TUNER_CHNZ_0_FE_BYPHA_MASK                         0x00001000
#define BCHP_DS_TUNER_CHNZ_0_FE_BYPHA_SHIFT                        12

/* DS_TUNER_CHNZ_0 :: FE :: BYPHB [11:11] */
#define BCHP_DS_TUNER_CHNZ_0_FE_BYPHB_MASK                         0x00000800
#define BCHP_DS_TUNER_CHNZ_0_FE_BYPHB_SHIFT                        11

/* DS_TUNER_CHNZ_0 :: FE :: BYPHC [10:10] */
#define BCHP_DS_TUNER_CHNZ_0_FE_BYPHC_MASK                         0x00000400
#define BCHP_DS_TUNER_CHNZ_0_FE_BYPHC_SHIFT                        10

/* DS_TUNER_CHNZ_0 :: FE :: reserved_for_eco0 [09:08] */
#define BCHP_DS_TUNER_CHNZ_0_FE_reserved_for_eco0_MASK             0x00000300
#define BCHP_DS_TUNER_CHNZ_0_FE_reserved_for_eco0_SHIFT            8

/* DS_TUNER_CHNZ_0 :: FE :: SIGRST [07:07] */
#define BCHP_DS_TUNER_CHNZ_0_FE_SIGRST_MASK                        0x00000080
#define BCHP_DS_TUNER_CHNZ_0_FE_SIGRST_SHIFT                       7

/* DS_TUNER_CHNZ_0 :: FE :: SIGEN [06:06] */
#define BCHP_DS_TUNER_CHNZ_0_FE_SIGEN_MASK                         0x00000040
#define BCHP_DS_TUNER_CHNZ_0_FE_SIGEN_SHIFT                        6

/* DS_TUNER_CHNZ_0 :: FE :: IQ_TPIN_SEL [05:05] */
#define BCHP_DS_TUNER_CHNZ_0_FE_IQ_TPIN_SEL_MASK                   0x00000020
#define BCHP_DS_TUNER_CHNZ_0_FE_IQ_TPIN_SEL_SHIFT                  5

/* DS_TUNER_CHNZ_0 :: FE :: INPSEL [04:04] */
#define BCHP_DS_TUNER_CHNZ_0_FE_INPSEL_MASK                        0x00000010
#define BCHP_DS_TUNER_CHNZ_0_FE_INPSEL_SHIFT                       4

/* DS_TUNER_CHNZ_0 :: FE :: ZERO_INPUT [03:03] */
#define BCHP_DS_TUNER_CHNZ_0_FE_ZERO_INPUT_MASK                    0x00000008
#define BCHP_DS_TUNER_CHNZ_0_FE_ZERO_INPUT_SHIFT                   3

/* DS_TUNER_CHNZ_0 :: FE :: reserved1 [02:02] */
#define BCHP_DS_TUNER_CHNZ_0_FE_reserved1_MASK                     0x00000004
#define BCHP_DS_TUNER_CHNZ_0_FE_reserved1_SHIFT                    2

/* DS_TUNER_CHNZ_0 :: FE :: MIXBYP [01:01] */
#define BCHP_DS_TUNER_CHNZ_0_FE_MIXBYP_MASK                        0x00000002
#define BCHP_DS_TUNER_CHNZ_0_FE_MIXBYP_SHIFT                       1

/* DS_TUNER_CHNZ_0 :: FE :: reserved_for_eco2 [00:00] */
#define BCHP_DS_TUNER_CHNZ_0_FE_reserved_for_eco2_MASK             0x00000001
#define BCHP_DS_TUNER_CHNZ_0_FE_reserved_for_eco2_SHIFT            0

/***************************************************************************
 *SGFE - Front end Signature Analyzer
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: SGFE :: SIGAVAL [31:00] */
#define BCHP_DS_TUNER_CHNZ_0_SGFE_SIGAVAL_MASK                     0xffffffff
#define BCHP_DS_TUNER_CHNZ_0_SGFE_SIGAVAL_SHIFT                    0

/***************************************************************************
 *ADC - Current ADC Output Sample
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: ADC :: reserved0 [31:28] */
#define BCHP_DS_TUNER_CHNZ_0_ADC_reserved0_MASK                    0xf0000000
#define BCHP_DS_TUNER_CHNZ_0_ADC_reserved0_SHIFT                   28

/* DS_TUNER_CHNZ_0 :: ADC :: ADCSAMP [27:16] */
#define BCHP_DS_TUNER_CHNZ_0_ADC_ADCSAMP_MASK                      0x0fff0000
#define BCHP_DS_TUNER_CHNZ_0_ADC_ADCSAMP_SHIFT                     16

/* DS_TUNER_CHNZ_0 :: ADC :: reserved1 [15:12] */
#define BCHP_DS_TUNER_CHNZ_0_ADC_reserved1_MASK                    0x0000f000
#define BCHP_DS_TUNER_CHNZ_0_ADC_reserved1_SHIFT                   12

/* DS_TUNER_CHNZ_0 :: ADC :: ADCSAMP_Q [11:00] */
#define BCHP_DS_TUNER_CHNZ_0_ADC_ADCSAMP_Q_MASK                    0x00000fff
#define BCHP_DS_TUNER_CHNZ_0_ADC_ADCSAMP_Q_SHIFT                   0

/***************************************************************************
 *AGF - Digital AGC(Fine) Control Register
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: AGF :: reserved0 [31:28] */
#define BCHP_DS_TUNER_CHNZ_0_AGF_reserved0_MASK                    0xf0000000
#define BCHP_DS_TUNER_CHNZ_0_AGF_reserved0_SHIFT                   28

/* DS_TUNER_CHNZ_0 :: AGF :: AGFBW [27:24] */
#define BCHP_DS_TUNER_CHNZ_0_AGF_AGFBW_MASK                        0x0f000000
#define BCHP_DS_TUNER_CHNZ_0_AGF_AGFBW_SHIFT                       24

/* DS_TUNER_CHNZ_0 :: AGF :: AGFTHR [23:08] */
#define BCHP_DS_TUNER_CHNZ_0_AGF_AGFTHR_MASK                       0x00ffff00
#define BCHP_DS_TUNER_CHNZ_0_AGF_AGFTHR_SHIFT                      8

/* DS_TUNER_CHNZ_0 :: AGF :: reserved1 [07:02] */
#define BCHP_DS_TUNER_CHNZ_0_AGF_reserved1_MASK                    0x000000fc
#define BCHP_DS_TUNER_CHNZ_0_AGF_reserved1_SHIFT                   2

/* DS_TUNER_CHNZ_0 :: AGF :: AGFBYP [01:01] */
#define BCHP_DS_TUNER_CHNZ_0_AGF_AGFBYP_MASK                       0x00000002
#define BCHP_DS_TUNER_CHNZ_0_AGF_AGFBYP_SHIFT                      1

/* DS_TUNER_CHNZ_0 :: AGF :: AGFRST [00:00] */
#define BCHP_DS_TUNER_CHNZ_0_AGF_AGFRST_MASK                       0x00000001
#define BCHP_DS_TUNER_CHNZ_0_AGF_AGFRST_SHIFT                      0

/***************************************************************************
 *AGFI - Digital AGC(Fine) Integrator Value
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: AGFI :: reserved0 [31:24] */
#define BCHP_DS_TUNER_CHNZ_0_AGFI_reserved0_MASK                   0xff000000
#define BCHP_DS_TUNER_CHNZ_0_AGFI_reserved0_SHIFT                  24

/* DS_TUNER_CHNZ_0 :: AGFI :: AGFVAL [23:00] */
#define BCHP_DS_TUNER_CHNZ_0_AGFI_AGFVAL_MASK                      0x00ffffff
#define BCHP_DS_TUNER_CHNZ_0_AGFI_AGFVAL_SHIFT                     0

/***************************************************************************
 *AGFLI - Digital AGC(Fine) Leaky Integrator Value
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: AGFLI :: reserved0 [31:16] */
#define BCHP_DS_TUNER_CHNZ_0_AGFLI_reserved0_MASK                  0xffff0000
#define BCHP_DS_TUNER_CHNZ_0_AGFLI_reserved0_SHIFT                 16

/* DS_TUNER_CHNZ_0 :: AGFLI :: AGFLVAL [15:00] */
#define BCHP_DS_TUNER_CHNZ_0_AGFLI_AGFLVAL_MASK                    0x0000ffff
#define BCHP_DS_TUNER_CHNZ_0_AGFLI_AGFLVAL_SHIFT                   0

/***************************************************************************
 *CFLOS - Carrier Frequency Loop Cosine/Sine
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: CFLOS :: CFL_SEL [31:31] */
#define BCHP_DS_TUNER_CHNZ_0_CFLOS_CFL_SEL_MASK                    0x80000000
#define BCHP_DS_TUNER_CHNZ_0_CFLOS_CFL_SEL_SHIFT                   31

/* DS_TUNER_CHNZ_0 :: CFLOS :: reserved0 [30:28] */
#define BCHP_DS_TUNER_CHNZ_0_CFLOS_reserved0_MASK                  0x70000000
#define BCHP_DS_TUNER_CHNZ_0_CFLOS_reserved0_SHIFT                 28

/* DS_TUNER_CHNZ_0 :: CFLOS :: CFL_SINE_OS [27:16] */
#define BCHP_DS_TUNER_CHNZ_0_CFLOS_CFL_SINE_OS_MASK                0x0fff0000
#define BCHP_DS_TUNER_CHNZ_0_CFLOS_CFL_SINE_OS_SHIFT               16

/* DS_TUNER_CHNZ_0 :: CFLOS :: reserved1 [15:12] */
#define BCHP_DS_TUNER_CHNZ_0_CFLOS_reserved1_MASK                  0x0000f000
#define BCHP_DS_TUNER_CHNZ_0_CFLOS_reserved1_SHIFT                 12

/* DS_TUNER_CHNZ_0 :: CFLOS :: CFL_COSINE_OS [11:00] */
#define BCHP_DS_TUNER_CHNZ_0_CFLOS_CFL_COSINE_OS_MASK              0x00000fff
#define BCHP_DS_TUNER_CHNZ_0_CFLOS_CFL_COSINE_OS_SHIFT             0

/***************************************************************************
 *DPM - DPM register for control signals
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: DPM :: reserved0 [31:30] */
#define BCHP_DS_TUNER_CHNZ_0_DPM_reserved0_MASK                    0xc0000000
#define BCHP_DS_TUNER_CHNZ_0_DPM_reserved0_SHIFT                   30

/* DS_TUNER_CHNZ_0 :: DPM :: DPMDCBYP [29:29] */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DPMDCBYP_MASK                     0x20000000
#define BCHP_DS_TUNER_CHNZ_0_DPM_DPMDCBYP_SHIFT                    29

/* DS_TUNER_CHNZ_0 :: DPM :: DPMCMULTBYP [28:28] */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DPMCMULTBYP_MASK                  0x10000000
#define BCHP_DS_TUNER_CHNZ_0_DPM_DPMCMULTBYP_SHIFT                 28

/* DS_TUNER_CHNZ_0 :: DPM :: DPMFCW [27:04] */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DPMFCW_MASK                       0x0ffffff0
#define BCHP_DS_TUNER_CHNZ_0_DPM_DPMFCW_SHIFT                      4

/* DS_TUNER_CHNZ_0 :: DPM :: DPMDCRST [03:03] */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DPMDCRST_MASK                     0x00000008
#define BCHP_DS_TUNER_CHNZ_0_DPM_DPMDCRST_SHIFT                    3

/* DS_TUNER_CHNZ_0 :: DPM :: DPMNOTCHBWC [02:00] */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DPMNOTCHBWC_MASK                  0x00000007
#define BCHP_DS_TUNER_CHNZ_0_DPM_DPMNOTCHBWC_SHIFT                 0

/***************************************************************************
 *DPM_DCI - DPM DC Canceller I Integrator Value
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: DPM_DCI :: DCIVAL [31:00] */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCI_DCIVAL_MASK                   0xffffffff
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCI_DCIVAL_SHIFT                  0

/***************************************************************************
 *DPM_DCQ - DPM DC Canceller Q Integrator Value
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: DPM_DCQ :: DCQVAL [31:00] */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCQ_DCQVAL_MASK                   0xffffffff
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCQ_DCQVAL_SHIFT                  0

/***************************************************************************
 *DPMK2 - DPM register for control signals
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: DPMK2 :: reserved0 [31:03] */
#define BCHP_DS_TUNER_CHNZ_0_DPMK2_reserved0_MASK                  0xfffffff8
#define BCHP_DS_TUNER_CHNZ_0_DPMK2_reserved0_SHIFT                 3

/* DS_TUNER_CHNZ_0 :: DPMK2 :: DPMK2DCRST [02:02] */
#define BCHP_DS_TUNER_CHNZ_0_DPMK2_DPMK2DCRST_MASK                 0x00000004
#define BCHP_DS_TUNER_CHNZ_0_DPMK2_DPMK2DCRST_SHIFT                2

/* DS_TUNER_CHNZ_0 :: DPMK2 :: DPMK2NOTCHBWC [01:00] */
#define BCHP_DS_TUNER_CHNZ_0_DPMK2_DPMK2NOTCHBWC_MASK              0x00000003
#define BCHP_DS_TUNER_CHNZ_0_DPMK2_DPMK2NOTCHBWC_SHIFT             0

/***************************************************************************
 *DPM_DCK2I - DPM DC Canceller I Integrator Value
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: DPM_DCK2I :: DCIVAL [31:00] */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCK2I_DCIVAL_MASK                 0xffffffff
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCK2I_DCIVAL_SHIFT                0

/***************************************************************************
 *DPM_DCK2Q - DPM DC Canceller Q Integrator Value
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: DPM_DCK2Q :: DCQVAL [31:00] */
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCK2Q_DCQVAL_MASK                 0xffffffff
#define BCHP_DS_TUNER_CHNZ_0_DPM_DCK2Q_DCQVAL_SHIFT                0

/***************************************************************************
 *FILTCCOEF_%i - Front End Programmable Filter C Coefficient Registers
 ***************************************************************************/
#define BCHP_DS_TUNER_CHNZ_0_FILTCCOEF_i_ARRAY_BASE                0x04c3c380
#define BCHP_DS_TUNER_CHNZ_0_FILTCCOEF_i_ARRAY_START               0
#define BCHP_DS_TUNER_CHNZ_0_FILTCCOEF_i_ARRAY_END                 31
#define BCHP_DS_TUNER_CHNZ_0_FILTCCOEF_i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *FILTCCOEF_%i - Front End Programmable Filter C Coefficient Registers
 ***************************************************************************/
/* DS_TUNER_CHNZ_0 :: FILTCCOEF_i :: reserved0 [31:12] */
#define BCHP_DS_TUNER_CHNZ_0_FILTCCOEF_i_reserved0_MASK            0xfffff000
#define BCHP_DS_TUNER_CHNZ_0_FILTCCOEF_i_reserved0_SHIFT           12

/* DS_TUNER_CHNZ_0 :: FILTCCOEF_i :: COEF [11:00] */
#define BCHP_DS_TUNER_CHNZ_0_FILTCCOEF_i_COEF_MASK                 0x00000fff
#define BCHP_DS_TUNER_CHNZ_0_FILTCCOEF_i_COEF_SHIFT                0


#endif /* #ifndef BCHP_DS_TUNER_CHNZ_0_H__ */

/* End of File */
