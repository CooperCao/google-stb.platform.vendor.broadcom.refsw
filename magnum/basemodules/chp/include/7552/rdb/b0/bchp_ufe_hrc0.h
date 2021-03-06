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
 * Date:           Generated on         Thu Aug  4 13:40:43 2011
 *                 MD5 Checksum         83f408cc25eb2d099cc58e22e4e239e9
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

#ifndef BCHP_UFE_HRC0_H__
#define BCHP_UFE_HRC0_H__

/***************************************************************************
 *UFE_HRC0 - UFE core registers HRC0
 ***************************************************************************/
#define BCHP_UFE_HRC0_CTRL                       0x00c00a00 /* clock/misc control register */
#define BCHP_UFE_HRC0_BYP                        0x00c00a04 /* bypass register */
#define BCHP_UFE_HRC0_RST                        0x00c00a08 /* reset control register */
#define BCHP_UFE_HRC0_FRZ                        0x00c00a0c /* freeze control register */
#define BCHP_UFE_HRC0_AGC1                       0x00c00a10 /* AGC1 control register */
#define BCHP_UFE_HRC0_AGC2                       0x00c00a14 /* AGC2 control register */
#define BCHP_UFE_HRC0_AGC3                       0x00c00a18 /* AGC3 control register */
#define BCHP_UFE_HRC0_AGC1_THRESH                0x00c00a1c /* AGC1 threshold register */
#define BCHP_UFE_HRC0_AGC2_THRESH                0x00c00a20 /* AGC2 threshold register */
#define BCHP_UFE_HRC0_AGC3_THRESH                0x00c00a24 /* AGC3 threshold register */
#define BCHP_UFE_HRC0_AGC1_LF                    0x00c00a28 /* AGC1 loop filter register */
#define BCHP_UFE_HRC0_AGC2_LF                    0x00c00a2c /* AGC2 loop filter register */
#define BCHP_UFE_HRC0_AGC3_LF                    0x00c00a30 /* AGC3 loop filter register */
#define BCHP_UFE_HRC0_IQIMB_AMP_CTRL             0x00c00a34 /* IQ-Imbalance amplitude correction control register */
#define BCHP_UFE_HRC0_IQIMB_PHS_CTRL             0x00c00a38 /* IQ-Imbalance phase     correction control register */
#define BCHP_UFE_HRC0_IQIMB_AMP_LF               0x00c00a3c /* IQ-Imbalance amplitude loop filter register */
#define BCHP_UFE_HRC0_IQIMB_PHS_LF               0x00c00a40 /* IQ-Imbalance phase     loop filter register */
#define BCHP_UFE_HRC0_DCO_CTRL                   0x00c00a44 /* DCO canceller control register */
#define BCHP_UFE_HRC0_DCOINTI                    0x00c00a48 /* DCO integrator I */
#define BCHP_UFE_HRC0_DCOINTQ                    0x00c00a4c /* DCO integrator Q */
#define BCHP_UFE_HRC0_BMIX_FCW                   0x00c00a50 /* FCW register for back mixer */
#define BCHP_UFE_HRC0_FMIX_FCW                   0x00c00a54 /* FCW register for front mixer */
#define BCHP_UFE_HRC0_CRC_EN                     0x00c00a58 /* CRC enable register */
#define BCHP_UFE_HRC0_CRC                        0x00c00a5c /* CRC signature analyzer register */
#define BCHP_UFE_HRC0_LFSR_SEED                  0x00c00a60 /* LFSR initial seed */
#define BCHP_UFE_HRC0_TP                         0x00c00a64 /* Testport register */
#define BCHP_UFE_HRC0_SPARE                      0x00c00a68 /* Software spare register */

#endif /* #ifndef BCHP_UFE_HRC0_H__ */

/* End of File */
