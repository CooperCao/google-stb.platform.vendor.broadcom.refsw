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
 * Date:           Generated on         Fri Apr 19 14:05:34 2013
 *                 MD5 Checksum         d41d8cd98f00b204e9800998ecf8427e
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

#ifndef BCHP_AFEC1_0_H__
#define BCHP_AFEC1_0_H__

/***************************************************************************
 *AFEC1_0 - 0 AFEC Register Set for Channel 1
 ***************************************************************************/
#define BCHP_AFEC1_0_RST                         0x01252000 /* AFEC core channel-based reset register */
#define BCHP_AFEC1_0_CNTR_CTRL                   0x01252004 /* AFEC counter-control register */
#define BCHP_AFEC1_0_MODCOD_PARAM_0              0x01252008 /* AFEC MODCOD parameter register */
#define BCHP_AFEC1_0_MODCOD_PARAM_1              0x0125200c /* AFEC MODCOD parameter register */
#define BCHP_AFEC1_0_MODCOD_STATS_CONFIG         0x01252010 /* MODCOD statistics configuration block */
#define BCHP_AFEC1_0_LDPC_ITER_CNT               0x01252014 /* LDPC Iteration counter */
#define BCHP_AFEC1_0_LDPC_FAIL_CNT               0x01252018 /* LDPC FAIL COUNTER */
#define BCHP_AFEC1_0_LDPC_FRM_CNT                0x0125201c /* LDPC FRAME COUNTER */
#define BCHP_AFEC1_0_LDPC_CONFIG                 0x01252020 /* LDPC Configuration Register */
#define BCHP_AFEC1_0_LDPC_STATUS                 0x01252024 /* LDPC Status Register */
#define BCHP_AFEC1_0_LDPC_MET_CRC                0x01252028 /* Metric Generator Signature */
#define BCHP_AFEC1_0_LDPC_EDGE_CRC               0x0125202c /* Edge Output Signature */
#define BCHP_AFEC1_0_LDPC_PSL_CTL                0x01252030 /* Power Saving Loop Control register */
#define BCHP_AFEC1_0_LDPC_PSL_INT_THRES          0x01252034 /* PSL Integrator Threshold */
#define BCHP_AFEC1_0_LDPC_PSL_INT                0x01252038 /* PSL Integrator Value */
#define BCHP_AFEC1_0_LDPC_PSL_AVE                0x0125203c /* PSL Integrator Average Value */
#define BCHP_AFEC1_0_LDPC_PSL_XCS                0x01252040 /* PSL Excess Value */
#define BCHP_AFEC1_0_LDPC_PSL_FILTER             0x01252044 /* PSL Filter for ACM */
#define BCHP_AFEC1_0_BCH_TPSIG                   0x01252050 /* BCH Block Signature Analyzer */
#define BCHP_AFEC1_0_BCH_SMTH_FIFO_MIN_LEVEL     0x01252054 /* BCH Smoother Fifo Min Level */
#define BCHP_AFEC1_0_BCH_SMTH_FIFO_MAX_LEVEL     0x01252058 /* BCH Smoother Fifo Max Level */
#define BCHP_AFEC1_0_BCH_CTRL                    0x01252064 /* BCH Decoder Control Register */
#define BCHP_AFEC1_0_BCH_DECNBLK                 0x01252068 /* BCH Number of Block Counter */
#define BCHP_AFEC1_0_BCH_DECCBLK                 0x0125206c /* BCH Number of Corrected Block Counter */
#define BCHP_AFEC1_0_BCH_DECBBLK                 0x01252070 /* BCH Number of Bad Block Counter */
#define BCHP_AFEC1_0_BCH_DECCBIT                 0x01252074 /* BCH Number of Corrected Bit Counter */
#define BCHP_AFEC1_0_BCH_DECMCOR                 0x01252078 /* BCH Number of Miscorrected Block Counter */
#define BCHP_AFEC1_0_BCH_BBHDR0                  0x0125207c /* BBHEADER Register 0 */
#define BCHP_AFEC1_0_BCH_BBHDR1                  0x01252080 /* BBHEADER Register 1 */
#define BCHP_AFEC1_0_BCH_BBHDR2                  0x01252084 /* BBHEADER Register 2 */
#define BCHP_AFEC1_0_BCH_BBHDR3                  0x01252088 /* BBHEADER Register 3 */
#define BCHP_AFEC1_0_BCH_BBHDR4                  0x0125208c /* BBHEADER Register 4 */
#define BCHP_AFEC1_0_BCH_BBHDR5                  0x012520c0 /* BBHEADER Register 5 */
#define BCHP_AFEC1_0_BCH_MPLCK                   0x012520c4 /* MPEG Lock Detector Configuration Register */
#define BCHP_AFEC1_0_BCH_MPCFG                   0x012520c8 /* MPEG Packetizer Configuration Register */
#define BCHP_AFEC1_0_BCH_SMCFG                   0x012520cc /* Smoother FIFO Configuration Register */
#define BCHP_AFEC1_0_CH_FRMCYCLES                0x01252a00 /* Channel frame period  Register */
#define BCHP_AFEC1_0_BIST_PARAM_0                0x01252a04 /* Channel-based BIST Configuration Register 0 */
#define BCHP_AFEC1_0_BIST_PARAM_1                0x01252a08 /* Channel-based BIST Configuration Register 1 */
#define BCHP_AFEC1_0_FAKEFRM_PARAM               0x01252a0c /* Parameters related to the fake frame generation during early termination */

#endif /* #ifndef BCHP_AFEC1_0_H__ */

/* End of File */
