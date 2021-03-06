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
 * Date:           Generated on              Thu Feb 20 15:33:52 2014
 *                 Full Compile MD5 Checksum 1766fea499add5f6ee91330ef96d35c5
 *                   (minus title and desc)
 *                 MD5 Checksum              4c358fb5b94802f03aec82d8df2c9afa
 *
 * Compiled with:  RDB Utility               combo_header.pl
 *                 RDB Parser                3.0
 *                 unknown                   unknown
 *                 Perl Interpreter          5.008008
 *                 Operating System          linux
 *
 *
 ***************************************************************************/



/****************************************************************************
 ***************************************************************************/

#ifndef BCHP_AFEC1_2_H__
#define BCHP_AFEC1_2_H__

/***************************************************************************
 *AFEC1_2 - AFEC Register Set for Channel 1
 ***************************************************************************/
#define BCHP_AFEC1_2_RST                         0x06202000 /* AFEC core channel-based reset register */
#define BCHP_AFEC1_2_CNTR_CTRL                   0x06202004 /* AFEC counter-control register */
#define BCHP_AFEC1_2_MODCOD_PARAM_0              0x06202008 /* AFEC MODCOD parameter register */
#define BCHP_AFEC1_2_MODCOD_PARAM_1              0x0620200c /* AFEC MODCOD parameter register */
#define BCHP_AFEC1_2_MODCOD_STATS_CONFIG         0x06202010 /* MODCOD statistics configuration block */
#define BCHP_AFEC1_2_LDPC_ITER_CNT               0x06202014 /* LDPC Iteration counter */
#define BCHP_AFEC1_2_LDPC_FAIL_CNT               0x06202018 /* LDPC FAIL COUNTER */
#define BCHP_AFEC1_2_LDPC_FRM_CNT                0x0620201c /* LDPC FRAME COUNTER */
#define BCHP_AFEC1_2_LDPC_CONFIG                 0x06202020 /* LDPC Configuration Register */
#define BCHP_AFEC1_2_LDPC_STATUS                 0x06202024 /* LDPC Status Register */
#define BCHP_AFEC1_2_LDPC_MET_CRC                0x06202028 /* Metric Generator Signature */
#define BCHP_AFEC1_2_LDPC_EDGE_CRC               0x0620202c /* Edge Output Signature */
#define BCHP_AFEC1_2_LDPC_PSL_CTL                0x06202030 /* Power Saving Loop Control register */
#define BCHP_AFEC1_2_LDPC_PSL_INT_THRES          0x06202034 /* PSL Integrator Threshold */
#define BCHP_AFEC1_2_LDPC_PSL_INT                0x06202038 /* PSL Integrator Value */
#define BCHP_AFEC1_2_LDPC_PSL_AVE                0x0620203c /* PSL Integrator Average Value */
#define BCHP_AFEC1_2_LDPC_PSL_XCS                0x06202040 /* PSL Excess Value */
#define BCHP_AFEC1_2_LDPC_PSL_FILTER             0x06202044 /* PSL Filter for ACM */
#define BCHP_AFEC1_2_BCH_TPSIG                   0x06202050 /* BCH Block Signature Analyzer */
#define BCHP_AFEC1_2_BCH_SMTH_FIFO_MIN_LEVEL     0x06202054 /* BCH Smoother Fifo Min Level */
#define BCHP_AFEC1_2_BCH_SMTH_FIFO_MAX_LEVEL     0x06202058 /* BCH Smoother Fifo Max Level */
#define BCHP_AFEC1_2_BCH_CTRL                    0x06202064 /* BCH Decoder Control Register */
#define BCHP_AFEC1_2_BCH_DECNBLK                 0x06202068 /* BCH Number of Block Counter */
#define BCHP_AFEC1_2_BCH_DECCBLK                 0x0620206c /* BCH Number of Corrected Block Counter */
#define BCHP_AFEC1_2_BCH_DECBBLK                 0x06202070 /* BCH Number of Bad Block Counter */
#define BCHP_AFEC1_2_BCH_DECCBIT                 0x06202074 /* BCH Number of Corrected Bit Counter */
#define BCHP_AFEC1_2_BCH_DECMCOR                 0x06202078 /* BCH Number of Miscorrected Block Counter */
#define BCHP_AFEC1_2_BCH_BBHDR0                  0x0620207c /* BBHEADER Register 0 */
#define BCHP_AFEC1_2_BCH_BBHDR1                  0x06202080 /* BBHEADER Register 1 */
#define BCHP_AFEC1_2_BCH_BBHDR2                  0x06202084 /* BBHEADER Register 2 */
#define BCHP_AFEC1_2_BCH_BBHDR3                  0x06202088 /* BBHEADER Register 3 */
#define BCHP_AFEC1_2_BCH_BBHDR4                  0x0620208c /* BBHEADER Register 4 */
#define BCHP_AFEC1_2_BCH_BBHDR5                  0x062020c0 /* BBHEADER Register 5 */
#define BCHP_AFEC1_2_BCH_MPLCK                   0x062020c4 /* MPEG Lock Detector Configuration Register */
#define BCHP_AFEC1_2_BCH_MPCFG                   0x062020c8 /* MPEG Packetizer Configuration Register */
#define BCHP_AFEC1_2_BCH_SMCFG                   0x062020cc /* Smoother FIFO Configuration Register */
#define BCHP_AFEC1_2_CH_FRMCYCLES                0x06202a00 /* Channel frame period  Register */
#define BCHP_AFEC1_2_BIST_PARAM_0                0x06202a04 /* Channel-based BIST Configuration Register 0 */
#define BCHP_AFEC1_2_BIST_PARAM_1                0x06202a08 /* Channel-based BIST Configuration Register 1 */
#define BCHP_AFEC1_2_FAKEFRM_PARAM               0x06202a0c /* Parameters related to the fake frame generation during early termination */

#endif /* #ifndef BCHP_AFEC1_2_H__ */

/* End of File */
