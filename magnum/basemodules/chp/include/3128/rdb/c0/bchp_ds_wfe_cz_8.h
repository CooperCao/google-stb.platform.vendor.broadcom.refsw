/***************************************************************************
 *     Copyright (c) 1999-2012, Broadcom Corporation
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
 * Date:           Generated on         Thu Oct  6 10:25:15 2011
 *                 MD5 Checksum         8f37dfc69866893136bc4603c1bedf8c
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

#ifndef BCHP_DS_WFE_CZ_8_H__
#define BCHP_DS_WFE_CZ_8_H__

/***************************************************************************
 *DS_WFE_CZ_8 - WFE Channel Processor 8 Configuration Registers
 ***************************************************************************/
#define BCHP_DS_WFE_CZ_8_SRC_ID                  0x0009c000 /* Source channel bin ID */
#define BCHP_DS_WFE_CZ_8_QMIX_FCW                0x0009c004 /* Channel Processor Mixer Frequency Control Word */
#define BCHP_DS_WFE_CZ_8_AGF                     0x0009c008 /* Digital AGC(Fine) Control Register */
#define BCHP_DS_WFE_CZ_8_AGFI                    0x0009c00c /* Digital AGC(Fine) Integrator Value */
#define BCHP_DS_WFE_CZ_8_AGFLI                   0x0009c010 /* Digital AGC(Fine) Leaky Integrator Value */


/***************************************************************************
 *SRC_ID - Source channel bin ID
 ***************************************************************************/
/* DS_WFE_CZ_8 :: SRC_ID :: BIN_ID [07:02] */
#define BCHP_DS_WFE_CZ_8_SRC_ID_BIN_ID_MASK                        0x000000fc
#define BCHP_DS_WFE_CZ_8_SRC_ID_BIN_ID_SHIFT                       2
#define BCHP_DS_WFE_CZ_8_SRC_ID_BIN_ID_DEFAULT                     0

/* DS_WFE_CZ_8 :: SRC_ID :: ACI_COEF_SEL [10:09] */
#define BCHP_DS_WFE_CZ_8_SRC_ID_ACI_COEF_SEL_MASK                  0x00000600
#define BCHP_DS_WFE_CZ_8_SRC_ID_ACI_COEF_SEL_SHIFT                 9
#define BCHP_DS_WFE_CZ_8_SRC_ID_ACI_COEF_SEL_DEFAULT               0

/* DS_WFE_CZ_8 :: SRC_ID :: FILTC_ODD_LEN [12:12] */
#define BCHP_DS_WFE_CZ_8_SRC_ID_FILTC_ODD_LEN_MASK                 0x00001000
#define BCHP_DS_WFE_CZ_8_SRC_ID_FILTC_ODD_LEN_SHIFT                12
#define BCHP_DS_WFE_CZ_8_SRC_ID_FILTC_ODD_LEN_DEFAULT              0


/***************************************************************************
 *QMIX_FCW - Channel Processor Mixer Frequency Control Word
 ***************************************************************************/
/* DS_WFE_CZ_8 :: QMIX_FCW :: reserved0 [31:30] */
#define BCHP_DS_WFE_CZ_8_QMIX_FCW_reserved0_MASK                   0xc0000000
#define BCHP_DS_WFE_CZ_8_QMIX_FCW_reserved0_SHIFT                  30

/* DS_WFE_CZ_8 :: QMIX_FCW :: FCW [29:00] */
#define BCHP_DS_WFE_CZ_8_QMIX_FCW_FCW_MASK                         0x3fffffff
#define BCHP_DS_WFE_CZ_8_QMIX_FCW_FCW_SHIFT                        0
#define BCHP_DS_WFE_CZ_8_QMIX_FCW_FCW_DEFAULT                      0

/***************************************************************************
 *AGF - Digital AGC(Fine) Control Register
 ***************************************************************************/
/* DS_WFE_CZ_8 :: AGF :: AGFBW [31:27] */
#define BCHP_DS_WFE_CZ_8_AGF_AGFBW_MASK                            0xf8000000
#define BCHP_DS_WFE_CZ_8_AGF_AGFBW_SHIFT                           27
#define BCHP_DS_WFE_CZ_8_AGF_AGFBW_DEFAULT                         7

/* DS_WFE_CZ_8 :: AGF :: AGFTHR [26:08] */
#define BCHP_DS_WFE_CZ_8_AGF_AGFTHR_MASK                           0x07ffff00
#define BCHP_DS_WFE_CZ_8_AGF_AGFTHR_SHIFT                          8
#define BCHP_DS_WFE_CZ_8_AGF_AGFTHR_DEFAULT                        65536

/* DS_WFE_CZ_8 :: AGF :: AGFRST [00:00] */
#define BCHP_DS_WFE_CZ_8_AGF_AGFRST_MASK                           0x00000001
#define BCHP_DS_WFE_CZ_8_AGF_AGFRST_SHIFT                          0
#define BCHP_DS_WFE_CZ_8_AGF_AGFRST_DEFAULT                        0

#endif /* #ifndef BCHP_DS_WFE_CZ_8_H__ */

/* End of File */
