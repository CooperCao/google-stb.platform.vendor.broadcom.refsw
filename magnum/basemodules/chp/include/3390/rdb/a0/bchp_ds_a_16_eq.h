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
 * Date:           Generated on               Mon Sep 15 10:12:19 2014
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

#ifndef BCHP_DS_A_16_EQ_H__
#define BCHP_DS_A_16_EQ_H__

/***************************************************************************
 *DS_A_16_EQ - Downstream Equalizer Registers
 ***************************************************************************/
#define BCHP_DS_A_16_EQ_CFL_CTL                  0x02240800 /* [RW] Carrier Frequency Loop Control Register */
#define BCHP_DS_A_16_EQ_CFL_INT                  0x02240804 /* [RW] Carrier Frequency Loop Integrator Register */
#define BCHP_DS_A_16_EQ_FFE_CTL1                 0x02240808 /* [RW] FFE Control Register1 */
#define BCHP_DS_A_16_EQ_FFE_CTL2                 0x0224080c /* [RW] FFE Control Register2 */
#define BCHP_DS_A_16_EQ_DFE_CTL1                 0x02240890 /* [RW] DFE Control Register1 */
#define BCHP_DS_A_16_EQ_DFE_CTL2                 0x02240894 /* [RW] DFE Control Register1 */
#define BCHP_DS_A_16_EQ_CWC1_CTL                 0x02240928 /* [RW] CWC1 Control Register */
#define BCHP_DS_A_16_EQ_CWC1_COEFFS              0x0224092c /* [RW] CWC1 Phase/Frequency Loop Coefficient Control Register */
#define BCHP_DS_A_16_EQ_CWC1_FCW                 0x02240930 /* [RW] CWC1 Phase/Frequency Loop Frequency Control Word Register */
#define BCHP_DS_A_16_EQ_CWC1_INT                 0x02240934 /* [RW] CWC1 Phase/Frequency Loop Integrator Register */
#define BCHP_DS_A_16_EQ_CWC1_EQ_COEFFS           0x02240938 /* [RW] CWC1 EQ Real and Imaginary Coefficient Register */
#define BCHP_DS_A_16_EQ_CWC2_CTL                 0x0224093c /* [RW] CWC2 Control Register */
#define BCHP_DS_A_16_EQ_CWC2_COEFFS              0x02240940 /* [RW] CWC2 Phase/Frequency Loop Coefficient Control Register */
#define BCHP_DS_A_16_EQ_CWC2_FCW                 0x02240944 /* [RW] CWC2 Phase/Frequency Loop Frequency Control Word Register */
#define BCHP_DS_A_16_EQ_CWC2_INT                 0x02240948 /* [RW] CWC2 Phase/Frequency Loop Integrator Register */
#define BCHP_DS_A_16_EQ_CWC2_EQ_COEFFS           0x0224094c /* [RW] CWC2 EQ Real and Imaginary Coefficient Register */
#define BCHP_DS_A_16_EQ_CWC3_CTL                 0x02240950 /* [RW] CWC3 Control Register */
#define BCHP_DS_A_16_EQ_CWC3_COEFFS              0x02240954 /* [RW] CWC3 Phase/Frequency Loop Coefficient Control Register */
#define BCHP_DS_A_16_EQ_CWC3_FCW                 0x02240958 /* [RW] CWC3 Phase/Frequency Loop Frequency Control Word Register */
#define BCHP_DS_A_16_EQ_CWC3_INT                 0x0224095c /* [RW] CWC3 Phase/Frequency Loop Integrator Register */
#define BCHP_DS_A_16_EQ_CWC3_EQ_COEFFS           0x02240960 /* [RW] CWC3 EQ Real and Imaginary Coefficient Register */
#define BCHP_DS_A_16_EQ_CWC4_CTL                 0x02240964 /* [RW] CWC4 Control Register */
#define BCHP_DS_A_16_EQ_CWC4_COEFFS              0x02240968 /* [RW] CWC4 Phase/Frequency Loop Coefficient Control Register */
#define BCHP_DS_A_16_EQ_CWC4_FCW                 0x0224096c /* [RW] CWC4 Phase/Frequency Loop Frequency Control Word Register */
#define BCHP_DS_A_16_EQ_CWC4_INT                 0x02240970 /* [RW] CWC4 Phase/Frequency Loop Integrator Register */
#define BCHP_DS_A_16_EQ_CWC4_EQ_COEFFS           0x02240974 /* [RW] CWC4 EQ Real and Imaginary Coefficient Register */
#define BCHP_DS_A_16_EQ_HUMAGC_CTL               0x02240978 /* [RW] HUMAGC Control Register */
#define BCHP_DS_A_16_EQ_HUMAGC_FN                0x0224097c /* [RW] HUMAGC FN Register */
#define BCHP_DS_A_16_EQ_HUMAGC_GACC              0x02240980 /* [RW] HUMAGC Gain Accumulator Register */
#define BCHP_DS_A_16_EQ_CPL_CTL                  0x02240984 /* [RW] Carrier Phase Loop Control Register */
#define BCHP_DS_A_16_EQ_CPL_COEFFS               0x02240988 /* [RW] Carrier Phase Loop Coefficient Register */
#define BCHP_DS_A_16_EQ_CPL_THRESH               0x0224098c /* [RW] Carrier Phase Loop Threshold Register */
#define BCHP_DS_A_16_EQ_CPL_INT                  0x02240990 /* [RW] Carrier Phase Loop Integrator Register */
#define BCHP_DS_A_16_EQ_CPL_PA                   0x02240994 /* [RW] Carrier Phase Loop Phase Accumulator */
#define BCHP_DS_A_16_EQ_SLC_CTL                  0x02240998 /* [RW] Slicer Control Register */
#define BCHP_DS_A_16_EQ_SLC_MOD                  0x0224099c /* [RW] External CMA and RCA Modulus Register */
#define BCHP_DS_A_16_EQ_SLC_THRESH_1             0x022409a0 /* [RW] Burst Noise Carrier Threshold and EQ Register */
#define BCHP_DS_A_16_EQ_SLC_THRESH_2             0x022409a4 /* [RW] Burst Noise SNR register */
#define BCHP_DS_A_16_EQ_SLC_SOFT                 0x022409a8 /* [RW] Slicer Soft Decision Register */
#define BCHP_DS_A_16_EQ_SNR_CTL1                 0x022409ac /* [RW] Slicer SNR Control 1 */
#define BCHP_DS_A_16_EQ_SNR_CTL2                 0x022409b0 /* [RW] Slicer SNR Control 2 */
#define BCHP_DS_A_16_EQ_SNR_ERR_INT              0x022409b4 /* [RW] SNR Error Integrator */
#define BCHP_DS_A_16_EQ_BND_CTL                  0x022409b8 /* [RW] BND Control Register */
#define BCHP_DS_A_16_EQ_BND_THRESH               0x022409bc /* [RW] BND Threshold Register */
#define BCHP_DS_A_16_EQ_FFT_DATA_CTL             0x022409c0 /* [RW] FFT DATA Control Register */
#define BCHP_DS_A_16_EQ_SW_SPARE1                0x022409c4 /* [RW] Reserved for software use */
#define BCHP_DS_A_16_EQ_SW_SPARE2                0x022409c8 /* [RW] Reserved for software use */

/***************************************************************************
 *FFE_COEFFS_%i - FFE Coefficient Register
 ***************************************************************************/
#define BCHP_DS_A_16_EQ_FFE_COEFFS_i_ARRAY_BASE                    0x02240810
#define BCHP_DS_A_16_EQ_FFE_COEFFS_i_ARRAY_START                   0
#define BCHP_DS_A_16_EQ_FFE_COEFFS_i_ARRAY_END                     31
#define BCHP_DS_A_16_EQ_FFE_COEFFS_i_ARRAY_ELEMENT_SIZE            32

/***************************************************************************
 *FFE_COEFFS_%i - FFE Coefficient Register
 ***************************************************************************/
/* DS_A_16_EQ :: FFE_COEFFS_i :: IMSB [31:16] */
#define BCHP_DS_A_16_EQ_FFE_COEFFS_i_IMSB_MASK                     0xffff0000
#define BCHP_DS_A_16_EQ_FFE_COEFFS_i_IMSB_SHIFT                    16
#define BCHP_DS_A_16_EQ_FFE_COEFFS_i_IMSB_DEFAULT                  0x00000000

/* DS_A_16_EQ :: FFE_COEFFS_i :: QMSB [15:00] */
#define BCHP_DS_A_16_EQ_FFE_COEFFS_i_QMSB_MASK                     0x0000ffff
#define BCHP_DS_A_16_EQ_FFE_COEFFS_i_QMSB_SHIFT                    0
#define BCHP_DS_A_16_EQ_FFE_COEFFS_i_QMSB_DEFAULT                  0x00000000


/***************************************************************************
 *DFE_COEFFS_%i - DFE Coefficient Register
 ***************************************************************************/
#define BCHP_DS_A_16_EQ_DFE_COEFFS_i_ARRAY_BASE                    0x02240898
#define BCHP_DS_A_16_EQ_DFE_COEFFS_i_ARRAY_START                   0
#define BCHP_DS_A_16_EQ_DFE_COEFFS_i_ARRAY_END                     35
#define BCHP_DS_A_16_EQ_DFE_COEFFS_i_ARRAY_ELEMENT_SIZE            32

/***************************************************************************
 *DFE_COEFFS_%i - DFE Coefficient Register
 ***************************************************************************/
/* DS_A_16_EQ :: DFE_COEFFS_i :: IMSB [31:16] */
#define BCHP_DS_A_16_EQ_DFE_COEFFS_i_IMSB_MASK                     0xffff0000
#define BCHP_DS_A_16_EQ_DFE_COEFFS_i_IMSB_SHIFT                    16
#define BCHP_DS_A_16_EQ_DFE_COEFFS_i_IMSB_DEFAULT                  0x00000000

/* DS_A_16_EQ :: DFE_COEFFS_i :: QMSB [15:00] */
#define BCHP_DS_A_16_EQ_DFE_COEFFS_i_QMSB_MASK                     0x0000ffff
#define BCHP_DS_A_16_EQ_DFE_COEFFS_i_QMSB_SHIFT                    0
#define BCHP_DS_A_16_EQ_DFE_COEFFS_i_QMSB_DEFAULT                  0x00000000


/***************************************************************************
 *HIST_%i - History Register
 ***************************************************************************/
#define BCHP_DS_A_16_EQ_HIST_i_ARRAY_BASE                          0x022409cc
#define BCHP_DS_A_16_EQ_HIST_i_ARRAY_START                         0
#define BCHP_DS_A_16_EQ_HIST_i_ARRAY_END                           124
#define BCHP_DS_A_16_EQ_HIST_i_ARRAY_ELEMENT_SIZE                  32

/***************************************************************************
 *HIST_%i - History Register
 ***************************************************************************/
/* DS_A_16_EQ :: HIST_i :: DATA [31:00] */
#define BCHP_DS_A_16_EQ_HIST_i_DATA_MASK                           0xffffffff
#define BCHP_DS_A_16_EQ_HIST_i_DATA_SHIFT                          0
#define BCHP_DS_A_16_EQ_HIST_i_DATA_DEFAULT                        0x00000000


#endif /* #ifndef BCHP_DS_A_16_EQ_H__ */

/* End of File */
