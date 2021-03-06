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
 * Date:           Generated on               Fri Feb 20 00:05:25 2015
 *                 Full Compile MD5 Checksum  f4a546a20d0bd1f244e0d6a139e85ce0
 *                     (minus title and desc)
 *                 MD5 Checksum               a9d9eeea3a1c30a122d08de69d07786c
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15715
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_H__
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_H__

/***************************************************************************
 *AUD_FMM_IOP_OUT_MS_STRM_CFG_1 - MS_OUT Streams Config Control
 ***************************************************************************/
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_ENABLE_STATUS 0x00cb8000 /* [RO] Enable Status Register */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_ENABLE_SET 0x00cb8004 /* [WO] Enable Set Register */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_ENABLE_CLEAR 0x00cb8008 /* [WO] Enable Clear Register */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_FCI_CFG0 0x00cb800c /* [RW] FCI configuration for Stream0 - SPDIF */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_FCI_CFG1 0x00cb8010 /* [RW] FCI configuration for Stream0 - MAI */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_FCI_CFG2 0x00cb8014 /* [RW] FCI configuration for Stream0 - MAI_Multi_0 */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_FCI_CFG3 0x00cb8018 /* [RW] FCI configuration for Stream0 - MAI_MUlti_1 */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_FCI_CFG4 0x00cb801c /* [RW] FCI configuration  for Stream0 - MAI_MUlti_2 */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_FCI_CFG5 0x00cb8020 /* [RW] FCI configuration  for Stream0 - MAI_MUlti_3 */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_FCI_CFG6 0x00cb8024 /* [RW] FCI configuration  for Stream0 - MAI_HBR */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_ROUTE 0x00cb802c /* [RW] Stream routing */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_SPDIF_CFG_0 0x00cb8060 /* [RW] SPDIF 0 formatter configuration */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_MAI_MULTI_GROUPING 0x00cb8068 /* [RW] Multi-channel MAI group */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_MAI_CFG 0x00cb8090 /* [RW] MAI formatter configuration */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_MAI_FORMAT 0x00cb8094 /* [RW] MAI format */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_MAI_CROSSBAR 0x00cb8098 /* [RW] MAI crossbar control */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_MCLK_CFG_SPDIF0 0x00cb8110 /* [RW] SPDIF 0 MCLK configuration */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_MCLK_CFG_MAI_MULTI 0x00cb8130 /* [RW] MAIM MCLK configuration */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_MCLK_CFG_FS0 0x00cb8160 /* [RW] Fs_0  MCLK configuration */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_SPDIF_MAI_OUT_MUX_SEL 0x00cb8170 /* [RW] OUTPUT_Mux_select for SPDIF/MAI */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_DIAG_CFG 0x00cb81fc /* [RW] Diagnostic output configuration */

/***************************************************************************
 *STREAM_CFG%i - Stream 0..10 configuration
 ***************************************************************************/
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_ARRAY_BASE  0x00cb8030
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_ARRAY_START 0
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_ARRAY_END   10
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *STREAM_CFG%i - Stream 0..10 configuration
 ***************************************************************************/
/* AUD_FMM_IOP_OUT_MS_STRM_CFG_1 :: STREAM_CFGi :: reserved0 [31:02] */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_reserved0_MASK 0xfffffffc
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_reserved0_SHIFT 2

/* AUD_FMM_IOP_OUT_MS_STRM_CFG_1 :: STREAM_CFGi :: WAIT_FOR_VALID [01:01] */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_WAIT_FOR_VALID_MASK 0x00000002
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_WAIT_FOR_VALID_SHIFT 1
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_WAIT_FOR_VALID_DEFAULT 0x00000000
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_WAIT_FOR_VALID_Holdoff_request 1
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_WAIT_FOR_VALID_Keep_requesting 0

/* AUD_FMM_IOP_OUT_MS_STRM_CFG_1 :: STREAM_CFGi :: IGNORE_FIRST_UNDERFLOW [00:00] */
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_IGNORE_FIRST_UNDERFLOW_MASK 0x00000001
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_IGNORE_FIRST_UNDERFLOW_SHIFT 0
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_IGNORE_FIRST_UNDERFLOW_DEFAULT 0x00000001
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_IGNORE_FIRST_UNDERFLOW_Ignore 1
#define BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_STREAM_CFGi_IGNORE_FIRST_UNDERFLOW_Dont_ignore 0


#endif /* #ifndef BCHP_AUD_FMM_IOP_OUT_MS_STRM_CFG_1_H__ */

/* End of File */
