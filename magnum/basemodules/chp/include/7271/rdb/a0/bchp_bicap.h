/****************************************************************************
 *     Copyright (c) 1999-2015, Broadcom Corporation
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
 * Date:           Generated on               Thu Jun 18 10:52:56 2015
 *                 Full Compile MD5 Checksum  32b78c1804e11666b824f2b9450a6228
 *                     (minus title and desc)
 *                 MD5 Checksum               3452ff65b8043c1c458e059705af3b49
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     16265
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_BICAP_H__
#define BCHP_BICAP_H__

/***************************************************************************
 *BICAP - Buffered Input Capture
 ***************************************************************************/
#define BCHP_BICAP_REVISION                      0x204176c0 /* [RO] BICAP Revision */
#define BCHP_BICAP_CONTROL0                      0x204176c4 /* [RW] BICAP CONTROL FOR INPUT 0 */
#define BCHP_BICAP_CONTROL1                      0x204176c8 /* [RW] BICAP CONTROL FOR INPUT 1 */
#define BCHP_BICAP_FILTER0                       0x204176cc /* [RW] BICAP FILTER REGISTER FOR INPUT 0 */
#define BCHP_BICAP_FILTER1                       0x204176d0 /* [RW] BICAP FILTER REGISTER FOR INPUT 1 */
#define BCHP_BICAP_TIMEOUT0                      0x204176d4 /* [RW] TIMEOUT REGISTER 0 (Timeout Code: 0xFFF) */
#define BCHP_BICAP_TIMEOUT1                      0x204176d8 /* [RW] TIMEOUT REGISTER 1 (Timeout Code: 0xFFE) */
#define BCHP_BICAP_TIMEOUT2                      0x204176dc /* [RW] TIMEOUT REGISTER 2 (Timeout Code: 0xFFD) */
#define BCHP_BICAP_TIMEOUT3                      0x204176e0 /* [RW] TIMEOUT REGISTER 3 (Timeout Code: 0xFFC) */
#define BCHP_BICAP_FIFO_INACT_TIMEOUT            0x204176e4 /* [RW] FIFO INACTIVITY TIMEOUT REGISTER */
#define BCHP_BICAP_FIFO_DATA                     0x204176e8 /* [RO] BICAP FIFO DATA REGISTER */
#define BCHP_BICAP_FIFO_CONTROL                  0x204176ec /* [RW] BICAP FIFO CONTROL */
#define BCHP_BICAP_STATUS                        0x204176f0 /* [RO] BICAP FIFO and TIMEOUT STATUS */
#define BCHP_BICAP_PWC0                          0x204176f4 /* [RW] BICAP PULSE WIDTH COUNTER VALUE FOR INPUT 0 */
#define BCHP_BICAP_PWC1                          0x204176f8 /* [RW] BICAP PULSE WIDTH COUNTER VALUE FOR INPUT 1 */

/***************************************************************************
 *REVISION - BICAP Revision
 ***************************************************************************/
/* BICAP :: REVISION :: reserved0 [31:16] */
#define BCHP_BICAP_REVISION_reserved0_MASK                         0xffff0000
#define BCHP_BICAP_REVISION_reserved0_SHIFT                        16

/* BICAP :: REVISION :: MAJOR [15:08] */
#define BCHP_BICAP_REVISION_MAJOR_MASK                             0x0000ff00
#define BCHP_BICAP_REVISION_MAJOR_SHIFT                            8
#define BCHP_BICAP_REVISION_MAJOR_DEFAULT                          0x00000001

/* BICAP :: REVISION :: MINOR [07:00] */
#define BCHP_BICAP_REVISION_MINOR_MASK                             0x000000ff
#define BCHP_BICAP_REVISION_MINOR_SHIFT                            0
#define BCHP_BICAP_REVISION_MINOR_DEFAULT                          0x00000001

/***************************************************************************
 *CONTROL0 - BICAP CONTROL FOR INPUT 0
 ***************************************************************************/
/* BICAP :: CONTROL0 :: tout_clk_div [31:24] */
#define BCHP_BICAP_CONTROL0_tout_clk_div_MASK                      0xff000000
#define BCHP_BICAP_CONTROL0_tout_clk_div_SHIFT                     24
#define BCHP_BICAP_CONTROL0_tout_clk_div_DEFAULT                   0x00000000

/* BICAP :: CONTROL0 :: reserved_for_eco0 [23:22] */
#define BCHP_BICAP_CONTROL0_reserved_for_eco0_MASK                 0x00c00000
#define BCHP_BICAP_CONTROL0_reserved_for_eco0_SHIFT                22
#define BCHP_BICAP_CONTROL0_reserved_for_eco0_DEFAULT              0x00000000

/* BICAP :: CONTROL0 :: sys_clk_div [21:08] */
#define BCHP_BICAP_CONTROL0_sys_clk_div_MASK                       0x003fff00
#define BCHP_BICAP_CONTROL0_sys_clk_div_SHIFT                      8
#define BCHP_BICAP_CONTROL0_sys_clk_div_DEFAULT                    0x0000000f

/* BICAP :: CONTROL0 :: reserved_for_eco1 [07:06] */
#define BCHP_BICAP_CONTROL0_reserved_for_eco1_MASK                 0x000000c0
#define BCHP_BICAP_CONTROL0_reserved_for_eco1_SHIFT                6
#define BCHP_BICAP_CONTROL0_reserved_for_eco1_DEFAULT              0x00000000

/* BICAP :: CONTROL0 :: cnt_mode [05:04] */
#define BCHP_BICAP_CONTROL0_cnt_mode_MASK                          0x00000030
#define BCHP_BICAP_CONTROL0_cnt_mode_SHIFT                         4
#define BCHP_BICAP_CONTROL0_cnt_mode_DEFAULT                       0x00000000

/* BICAP :: CONTROL0 :: pedgedet_en [03:03] */
#define BCHP_BICAP_CONTROL0_pedgedet_en_MASK                       0x00000008
#define BCHP_BICAP_CONTROL0_pedgedet_en_SHIFT                      3
#define BCHP_BICAP_CONTROL0_pedgedet_en_DEFAULT                    0x00000000

/* BICAP :: CONTROL0 :: nedgedet_en [02:02] */
#define BCHP_BICAP_CONTROL0_nedgedet_en_MASK                       0x00000004
#define BCHP_BICAP_CONTROL0_nedgedet_en_SHIFT                      2
#define BCHP_BICAP_CONTROL0_nedgedet_en_DEFAULT                    0x00000000

/* BICAP :: CONTROL0 :: insig_inv [01:01] */
#define BCHP_BICAP_CONTROL0_insig_inv_MASK                         0x00000002
#define BCHP_BICAP_CONTROL0_insig_inv_SHIFT                        1
#define BCHP_BICAP_CONTROL0_insig_inv_DEFAULT                      0x00000000

/* BICAP :: CONTROL0 :: bicap_en [00:00] */
#define BCHP_BICAP_CONTROL0_bicap_en_MASK                          0x00000001
#define BCHP_BICAP_CONTROL0_bicap_en_SHIFT                         0
#define BCHP_BICAP_CONTROL0_bicap_en_DEFAULT                       0x00000000

/***************************************************************************
 *CONTROL1 - BICAP CONTROL FOR INPUT 1
 ***************************************************************************/
/* BICAP :: CONTROL1 :: tout_clk_div [31:24] */
#define BCHP_BICAP_CONTROL1_tout_clk_div_MASK                      0xff000000
#define BCHP_BICAP_CONTROL1_tout_clk_div_SHIFT                     24
#define BCHP_BICAP_CONTROL1_tout_clk_div_DEFAULT                   0x00000000

/* BICAP :: CONTROL1 :: reserved_for_eco0 [23:22] */
#define BCHP_BICAP_CONTROL1_reserved_for_eco0_MASK                 0x00c00000
#define BCHP_BICAP_CONTROL1_reserved_for_eco0_SHIFT                22
#define BCHP_BICAP_CONTROL1_reserved_for_eco0_DEFAULT              0x00000000

/* BICAP :: CONTROL1 :: sys_clk_div [21:08] */
#define BCHP_BICAP_CONTROL1_sys_clk_div_MASK                       0x003fff00
#define BCHP_BICAP_CONTROL1_sys_clk_div_SHIFT                      8
#define BCHP_BICAP_CONTROL1_sys_clk_div_DEFAULT                    0x0000000f

/* BICAP :: CONTROL1 :: reserved_for_eco1 [07:06] */
#define BCHP_BICAP_CONTROL1_reserved_for_eco1_MASK                 0x000000c0
#define BCHP_BICAP_CONTROL1_reserved_for_eco1_SHIFT                6
#define BCHP_BICAP_CONTROL1_reserved_for_eco1_DEFAULT              0x00000000

/* BICAP :: CONTROL1 :: cnt_mode [05:04] */
#define BCHP_BICAP_CONTROL1_cnt_mode_MASK                          0x00000030
#define BCHP_BICAP_CONTROL1_cnt_mode_SHIFT                         4
#define BCHP_BICAP_CONTROL1_cnt_mode_DEFAULT                       0x00000000

/* BICAP :: CONTROL1 :: pedgedet_en [03:03] */
#define BCHP_BICAP_CONTROL1_pedgedet_en_MASK                       0x00000008
#define BCHP_BICAP_CONTROL1_pedgedet_en_SHIFT                      3
#define BCHP_BICAP_CONTROL1_pedgedet_en_DEFAULT                    0x00000000

/* BICAP :: CONTROL1 :: nedgedet_en [02:02] */
#define BCHP_BICAP_CONTROL1_nedgedet_en_MASK                       0x00000004
#define BCHP_BICAP_CONTROL1_nedgedet_en_SHIFT                      2
#define BCHP_BICAP_CONTROL1_nedgedet_en_DEFAULT                    0x00000000

/* BICAP :: CONTROL1 :: insig_inv [01:01] */
#define BCHP_BICAP_CONTROL1_insig_inv_MASK                         0x00000002
#define BCHP_BICAP_CONTROL1_insig_inv_SHIFT                        1
#define BCHP_BICAP_CONTROL1_insig_inv_DEFAULT                      0x00000000

/* BICAP :: CONTROL1 :: bicap_en [00:00] */
#define BCHP_BICAP_CONTROL1_bicap_en_MASK                          0x00000001
#define BCHP_BICAP_CONTROL1_bicap_en_SHIFT                         0
#define BCHP_BICAP_CONTROL1_bicap_en_DEFAULT                       0x00000000

/***************************************************************************
 *FILTER0 - BICAP FILTER REGISTER FOR INPUT 0
 ***************************************************************************/
/* BICAP :: FILTER0 :: reserved0 [31:10] */
#define BCHP_BICAP_FILTER0_reserved0_MASK                          0xfffffc00
#define BCHP_BICAP_FILTER0_reserved0_SHIFT                         10

/* BICAP :: FILTER0 :: filter_bypass [09:09] */
#define BCHP_BICAP_FILTER0_filter_bypass_MASK                      0x00000200
#define BCHP_BICAP_FILTER0_filter_bypass_SHIFT                     9
#define BCHP_BICAP_FILTER0_filter_bypass_DEFAULT                   0x00000000

/* BICAP :: FILTER0 :: filter_clk_sel [08:08] */
#define BCHP_BICAP_FILTER0_filter_clk_sel_MASK                     0x00000100
#define BCHP_BICAP_FILTER0_filter_clk_sel_SHIFT                    8
#define BCHP_BICAP_FILTER0_filter_clk_sel_DEFAULT                  0x00000000
#define BCHP_BICAP_FILTER0_filter_clk_sel_bicap_clk_filter         0
#define BCHP_BICAP_FILTER0_filter_clk_sel_sys_clk_filter           1

/* BICAP :: FILTER0 :: reserved_for_eco1 [07:07] */
#define BCHP_BICAP_FILTER0_reserved_for_eco1_MASK                  0x00000080
#define BCHP_BICAP_FILTER0_reserved_for_eco1_SHIFT                 7
#define BCHP_BICAP_FILTER0_reserved_for_eco1_DEFAULT               0x00000000

/* union - case bicap_clk_sel [06:00] */
/* BICAP :: FILTER0 :: bicap_clk_sel :: filter [06:00] */
#define BCHP_BICAP_FILTER0_bicap_clk_sel_filter_MASK               0x0000007f
#define BCHP_BICAP_FILTER0_bicap_clk_sel_filter_SHIFT              0
#define BCHP_BICAP_FILTER0_bicap_clk_sel_filter_DEFAULT            0x00000000

/* union - case sys_clk_sel [06:00] */
/* BICAP :: FILTER0 :: sys_clk_sel :: filter [06:00] */
#define BCHP_BICAP_FILTER0_sys_clk_sel_filter_MASK                 0x0000007f
#define BCHP_BICAP_FILTER0_sys_clk_sel_filter_SHIFT                0
#define BCHP_BICAP_FILTER0_sys_clk_sel_filter_DEFAULT              0x00000000

/***************************************************************************
 *FILTER1 - BICAP FILTER REGISTER FOR INPUT 1
 ***************************************************************************/
/* BICAP :: FILTER1 :: reserved0 [31:10] */
#define BCHP_BICAP_FILTER1_reserved0_MASK                          0xfffffc00
#define BCHP_BICAP_FILTER1_reserved0_SHIFT                         10

/* BICAP :: FILTER1 :: filter_bypass [09:09] */
#define BCHP_BICAP_FILTER1_filter_bypass_MASK                      0x00000200
#define BCHP_BICAP_FILTER1_filter_bypass_SHIFT                     9
#define BCHP_BICAP_FILTER1_filter_bypass_DEFAULT                   0x00000000

/* BICAP :: FILTER1 :: filter_clk_sel [08:08] */
#define BCHP_BICAP_FILTER1_filter_clk_sel_MASK                     0x00000100
#define BCHP_BICAP_FILTER1_filter_clk_sel_SHIFT                    8
#define BCHP_BICAP_FILTER1_filter_clk_sel_DEFAULT                  0x00000000
#define BCHP_BICAP_FILTER1_filter_clk_sel_bicap_clk_filter         0
#define BCHP_BICAP_FILTER1_filter_clk_sel_sys_clk_filter           1

/* BICAP :: FILTER1 :: reserved_for_eco1 [07:07] */
#define BCHP_BICAP_FILTER1_reserved_for_eco1_MASK                  0x00000080
#define BCHP_BICAP_FILTER1_reserved_for_eco1_SHIFT                 7
#define BCHP_BICAP_FILTER1_reserved_for_eco1_DEFAULT               0x00000000

/* union - case bicap_clk_sel [06:00] */
/* BICAP :: FILTER1 :: bicap_clk_sel :: filter [06:00] */
#define BCHP_BICAP_FILTER1_bicap_clk_sel_filter_MASK               0x0000007f
#define BCHP_BICAP_FILTER1_bicap_clk_sel_filter_SHIFT              0
#define BCHP_BICAP_FILTER1_bicap_clk_sel_filter_DEFAULT            0x00000000

/* union - case sys_clk_sel [06:00] */
/* BICAP :: FILTER1 :: sys_clk_sel :: filter [06:00] */
#define BCHP_BICAP_FILTER1_sys_clk_sel_filter_MASK                 0x0000007f
#define BCHP_BICAP_FILTER1_sys_clk_sel_filter_SHIFT                0
#define BCHP_BICAP_FILTER1_sys_clk_sel_filter_DEFAULT              0x00000000

/***************************************************************************
 *TIMEOUT0 - TIMEOUT REGISTER 0 (Timeout Code: 0xFFF)
 ***************************************************************************/
/* BICAP :: TIMEOUT0 :: reserved0 [31:20] */
#define BCHP_BICAP_TIMEOUT0_reserved0_MASK                         0xfff00000
#define BCHP_BICAP_TIMEOUT0_reserved0_SHIFT                        20

/* BICAP :: TIMEOUT0 :: input_sel [19:19] */
#define BCHP_BICAP_TIMEOUT0_input_sel_MASK                         0x00080000
#define BCHP_BICAP_TIMEOUT0_input_sel_SHIFT                        19
#define BCHP_BICAP_TIMEOUT0_input_sel_DEFAULT                      0x00000000

/* BICAP :: TIMEOUT0 :: edge_sel [18:17] */
#define BCHP_BICAP_TIMEOUT0_edge_sel_MASK                          0x00060000
#define BCHP_BICAP_TIMEOUT0_edge_sel_SHIFT                         17
#define BCHP_BICAP_TIMEOUT0_edge_sel_DEFAULT                       0x00000000

/* BICAP :: TIMEOUT0 :: clk_sel [16:16] */
#define BCHP_BICAP_TIMEOUT0_clk_sel_MASK                           0x00010000
#define BCHP_BICAP_TIMEOUT0_clk_sel_SHIFT                          16
#define BCHP_BICAP_TIMEOUT0_clk_sel_DEFAULT                        0x00000000

/* BICAP :: TIMEOUT0 :: reserved1 [15:12] */
#define BCHP_BICAP_TIMEOUT0_reserved1_MASK                         0x0000f000
#define BCHP_BICAP_TIMEOUT0_reserved1_SHIFT                        12

/* BICAP :: TIMEOUT0 :: tout [11:00] */
#define BCHP_BICAP_TIMEOUT0_tout_MASK                              0x00000fff
#define BCHP_BICAP_TIMEOUT0_tout_SHIFT                             0
#define BCHP_BICAP_TIMEOUT0_tout_DEFAULT                           0x00000000

/***************************************************************************
 *TIMEOUT1 - TIMEOUT REGISTER 1 (Timeout Code: 0xFFE)
 ***************************************************************************/
/* BICAP :: TIMEOUT1 :: reserved0 [31:20] */
#define BCHP_BICAP_TIMEOUT1_reserved0_MASK                         0xfff00000
#define BCHP_BICAP_TIMEOUT1_reserved0_SHIFT                        20

/* BICAP :: TIMEOUT1 :: input_sel [19:19] */
#define BCHP_BICAP_TIMEOUT1_input_sel_MASK                         0x00080000
#define BCHP_BICAP_TIMEOUT1_input_sel_SHIFT                        19
#define BCHP_BICAP_TIMEOUT1_input_sel_DEFAULT                      0x00000000

/* BICAP :: TIMEOUT1 :: edge_sel [18:17] */
#define BCHP_BICAP_TIMEOUT1_edge_sel_MASK                          0x00060000
#define BCHP_BICAP_TIMEOUT1_edge_sel_SHIFT                         17
#define BCHP_BICAP_TIMEOUT1_edge_sel_DEFAULT                       0x00000000

/* BICAP :: TIMEOUT1 :: clk_sel [16:16] */
#define BCHP_BICAP_TIMEOUT1_clk_sel_MASK                           0x00010000
#define BCHP_BICAP_TIMEOUT1_clk_sel_SHIFT                          16
#define BCHP_BICAP_TIMEOUT1_clk_sel_DEFAULT                        0x00000000

/* BICAP :: TIMEOUT1 :: reserved1 [15:12] */
#define BCHP_BICAP_TIMEOUT1_reserved1_MASK                         0x0000f000
#define BCHP_BICAP_TIMEOUT1_reserved1_SHIFT                        12

/* BICAP :: TIMEOUT1 :: tout [11:00] */
#define BCHP_BICAP_TIMEOUT1_tout_MASK                              0x00000fff
#define BCHP_BICAP_TIMEOUT1_tout_SHIFT                             0
#define BCHP_BICAP_TIMEOUT1_tout_DEFAULT                           0x00000000

/***************************************************************************
 *TIMEOUT2 - TIMEOUT REGISTER 2 (Timeout Code: 0xFFD)
 ***************************************************************************/
/* BICAP :: TIMEOUT2 :: reserved0 [31:20] */
#define BCHP_BICAP_TIMEOUT2_reserved0_MASK                         0xfff00000
#define BCHP_BICAP_TIMEOUT2_reserved0_SHIFT                        20

/* BICAP :: TIMEOUT2 :: input_sel [19:19] */
#define BCHP_BICAP_TIMEOUT2_input_sel_MASK                         0x00080000
#define BCHP_BICAP_TIMEOUT2_input_sel_SHIFT                        19
#define BCHP_BICAP_TIMEOUT2_input_sel_DEFAULT                      0x00000000

/* BICAP :: TIMEOUT2 :: edge_sel [18:17] */
#define BCHP_BICAP_TIMEOUT2_edge_sel_MASK                          0x00060000
#define BCHP_BICAP_TIMEOUT2_edge_sel_SHIFT                         17
#define BCHP_BICAP_TIMEOUT2_edge_sel_DEFAULT                       0x00000000

/* BICAP :: TIMEOUT2 :: clk_sel [16:16] */
#define BCHP_BICAP_TIMEOUT2_clk_sel_MASK                           0x00010000
#define BCHP_BICAP_TIMEOUT2_clk_sel_SHIFT                          16
#define BCHP_BICAP_TIMEOUT2_clk_sel_DEFAULT                        0x00000000

/* BICAP :: TIMEOUT2 :: reserved1 [15:12] */
#define BCHP_BICAP_TIMEOUT2_reserved1_MASK                         0x0000f000
#define BCHP_BICAP_TIMEOUT2_reserved1_SHIFT                        12

/* BICAP :: TIMEOUT2 :: tout [11:00] */
#define BCHP_BICAP_TIMEOUT2_tout_MASK                              0x00000fff
#define BCHP_BICAP_TIMEOUT2_tout_SHIFT                             0
#define BCHP_BICAP_TIMEOUT2_tout_DEFAULT                           0x00000000

/***************************************************************************
 *TIMEOUT3 - TIMEOUT REGISTER 3 (Timeout Code: 0xFFC)
 ***************************************************************************/
/* BICAP :: TIMEOUT3 :: reserved0 [31:20] */
#define BCHP_BICAP_TIMEOUT3_reserved0_MASK                         0xfff00000
#define BCHP_BICAP_TIMEOUT3_reserved0_SHIFT                        20

/* BICAP :: TIMEOUT3 :: input_sel [19:19] */
#define BCHP_BICAP_TIMEOUT3_input_sel_MASK                         0x00080000
#define BCHP_BICAP_TIMEOUT3_input_sel_SHIFT                        19
#define BCHP_BICAP_TIMEOUT3_input_sel_DEFAULT                      0x00000000

/* BICAP :: TIMEOUT3 :: edge_sel [18:17] */
#define BCHP_BICAP_TIMEOUT3_edge_sel_MASK                          0x00060000
#define BCHP_BICAP_TIMEOUT3_edge_sel_SHIFT                         17
#define BCHP_BICAP_TIMEOUT3_edge_sel_DEFAULT                       0x00000000

/* BICAP :: TIMEOUT3 :: clk_sel [16:16] */
#define BCHP_BICAP_TIMEOUT3_clk_sel_MASK                           0x00010000
#define BCHP_BICAP_TIMEOUT3_clk_sel_SHIFT                          16
#define BCHP_BICAP_TIMEOUT3_clk_sel_DEFAULT                        0x00000000

/* BICAP :: TIMEOUT3 :: reserved1 [15:12] */
#define BCHP_BICAP_TIMEOUT3_reserved1_MASK                         0x0000f000
#define BCHP_BICAP_TIMEOUT3_reserved1_SHIFT                        12

/* BICAP :: TIMEOUT3 :: tout [11:00] */
#define BCHP_BICAP_TIMEOUT3_tout_MASK                              0x00000fff
#define BCHP_BICAP_TIMEOUT3_tout_SHIFT                             0
#define BCHP_BICAP_TIMEOUT3_tout_DEFAULT                           0x00000000

/***************************************************************************
 *FIFO_INACT_TIMEOUT - FIFO INACTIVITY TIMEOUT REGISTER
 ***************************************************************************/
/* BICAP :: FIFO_INACT_TIMEOUT :: reserved0 [31:20] */
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_reserved0_MASK               0xfff00000
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_reserved0_SHIFT              20

/* BICAP :: FIFO_INACT_TIMEOUT :: bicap_clk_sel [19:19] */
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_bicap_clk_sel_MASK           0x00080000
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_bicap_clk_sel_SHIFT          19
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_bicap_clk_sel_DEFAULT        0x00000000

/* BICAP :: FIFO_INACT_TIMEOUT :: event_sel [18:17] */
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_event_sel_MASK               0x00060000
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_event_sel_SHIFT              17
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_event_sel_DEFAULT            0x00000000

/* BICAP :: FIFO_INACT_TIMEOUT :: clk_sel [16:16] */
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_clk_sel_MASK                 0x00010000
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_clk_sel_SHIFT                16
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_clk_sel_DEFAULT              0x00000000

/* BICAP :: FIFO_INACT_TIMEOUT :: reserved1 [15:12] */
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_reserved1_MASK               0x0000f000
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_reserved1_SHIFT              12

/* BICAP :: FIFO_INACT_TIMEOUT :: tout [11:00] */
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_tout_MASK                    0x00000fff
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_tout_SHIFT                   0
#define BCHP_BICAP_FIFO_INACT_TIMEOUT_tout_DEFAULT                 0x00000000

/***************************************************************************
 *FIFO_DATA - BICAP FIFO DATA REGISTER
 ***************************************************************************/
/* BICAP :: FIFO_DATA :: reserved0 [31:14] */
#define BCHP_BICAP_FIFO_DATA_reserved0_MASK                        0xffffc000
#define BCHP_BICAP_FIFO_DATA_reserved0_SHIFT                       14

/* BICAP :: FIFO_DATA :: insig_source [13:13] */
#define BCHP_BICAP_FIFO_DATA_insig_source_MASK                     0x00002000
#define BCHP_BICAP_FIFO_DATA_insig_source_SHIFT                    13

/* BICAP :: FIFO_DATA :: edge_pol [12:12] */
#define BCHP_BICAP_FIFO_DATA_edge_pol_MASK                         0x00001000
#define BCHP_BICAP_FIFO_DATA_edge_pol_SHIFT                        12

/* BICAP :: FIFO_DATA :: data [11:00] */
#define BCHP_BICAP_FIFO_DATA_data_MASK                             0x00000fff
#define BCHP_BICAP_FIFO_DATA_data_SHIFT                            0

/***************************************************************************
 *FIFO_CONTROL - BICAP FIFO CONTROL
 ***************************************************************************/
/* BICAP :: FIFO_CONTROL :: reserved0 [31:14] */
#define BCHP_BICAP_FIFO_CONTROL_reserved0_MASK                     0xffffc000
#define BCHP_BICAP_FIFO_CONTROL_reserved0_SHIFT                    14

/* BICAP :: FIFO_CONTROL :: fifo_depth [13:08] */
#define BCHP_BICAP_FIFO_CONTROL_fifo_depth_MASK                    0x00003f00
#define BCHP_BICAP_FIFO_CONTROL_fifo_depth_SHIFT                   8

/* BICAP :: FIFO_CONTROL :: reserved1 [07:07] */
#define BCHP_BICAP_FIFO_CONTROL_reserved1_MASK                     0x00000080
#define BCHP_BICAP_FIFO_CONTROL_reserved1_SHIFT                    7

/* BICAP :: FIFO_CONTROL :: trig_lvl [06:01] */
#define BCHP_BICAP_FIFO_CONTROL_trig_lvl_MASK                      0x0000007e
#define BCHP_BICAP_FIFO_CONTROL_trig_lvl_SHIFT                     1
#define BCHP_BICAP_FIFO_CONTROL_trig_lvl_DEFAULT                   0x00000000

/* BICAP :: FIFO_CONTROL :: reset [00:00] */
#define BCHP_BICAP_FIFO_CONTROL_reset_MASK                         0x00000001
#define BCHP_BICAP_FIFO_CONTROL_reset_SHIFT                        0
#define BCHP_BICAP_FIFO_CONTROL_reset_DEFAULT                      0x00000000

/***************************************************************************
 *STATUS - BICAP FIFO and TIMEOUT STATUS
 ***************************************************************************/
/* BICAP :: STATUS :: reserved0 [31:16] */
#define BCHP_BICAP_STATUS_reserved0_MASK                           0xffff0000
#define BCHP_BICAP_STATUS_reserved0_SHIFT                          16

/* BICAP :: STATUS :: fifo_count [15:09] */
#define BCHP_BICAP_STATUS_fifo_count_MASK                          0x0000fe00
#define BCHP_BICAP_STATUS_fifo_count_SHIFT                         9
#define BCHP_BICAP_STATUS_fifo_count_DEFAULT                       0x00000000

/* BICAP :: STATUS :: fifo_full [08:08] */
#define BCHP_BICAP_STATUS_fifo_full_MASK                           0x00000100
#define BCHP_BICAP_STATUS_fifo_full_SHIFT                          8
#define BCHP_BICAP_STATUS_fifo_full_DEFAULT                        0x00000000

/* BICAP :: STATUS :: fifo_empty [07:07] */
#define BCHP_BICAP_STATUS_fifo_empty_MASK                          0x00000080
#define BCHP_BICAP_STATUS_fifo_empty_SHIFT                         7
#define BCHP_BICAP_STATUS_fifo_empty_DEFAULT                       0x00000001

/* BICAP :: STATUS :: fifo_overflow [06:06] */
#define BCHP_BICAP_STATUS_fifo_overflow_MASK                       0x00000040
#define BCHP_BICAP_STATUS_fifo_overflow_SHIFT                      6
#define BCHP_BICAP_STATUS_fifo_overflow_DEFAULT                    0x00000000

/* BICAP :: STATUS :: fifo_lvl_event [05:05] */
#define BCHP_BICAP_STATUS_fifo_lvl_event_MASK                      0x00000020
#define BCHP_BICAP_STATUS_fifo_lvl_event_SHIFT                     5
#define BCHP_BICAP_STATUS_fifo_lvl_event_DEFAULT                   0x00000000

/* BICAP :: STATUS :: fifo_inact_event [04:04] */
#define BCHP_BICAP_STATUS_fifo_inact_event_MASK                    0x00000010
#define BCHP_BICAP_STATUS_fifo_inact_event_SHIFT                   4
#define BCHP_BICAP_STATUS_fifo_inact_event_DEFAULT                 0x00000000

/* BICAP :: STATUS :: timeout3_event [03:03] */
#define BCHP_BICAP_STATUS_timeout3_event_MASK                      0x00000008
#define BCHP_BICAP_STATUS_timeout3_event_SHIFT                     3
#define BCHP_BICAP_STATUS_timeout3_event_DEFAULT                   0x00000000

/* BICAP :: STATUS :: timeout2_event [02:02] */
#define BCHP_BICAP_STATUS_timeout2_event_MASK                      0x00000004
#define BCHP_BICAP_STATUS_timeout2_event_SHIFT                     2
#define BCHP_BICAP_STATUS_timeout2_event_DEFAULT                   0x00000000

/* BICAP :: STATUS :: timeout1_event [01:01] */
#define BCHP_BICAP_STATUS_timeout1_event_MASK                      0x00000002
#define BCHP_BICAP_STATUS_timeout1_event_SHIFT                     1
#define BCHP_BICAP_STATUS_timeout1_event_DEFAULT                   0x00000000

/* BICAP :: STATUS :: timeout0_event [00:00] */
#define BCHP_BICAP_STATUS_timeout0_event_MASK                      0x00000001
#define BCHP_BICAP_STATUS_timeout0_event_SHIFT                     0
#define BCHP_BICAP_STATUS_timeout0_event_DEFAULT                   0x00000000

/***************************************************************************
 *PWC0 - BICAP PULSE WIDTH COUNTER VALUE FOR INPUT 0
 ***************************************************************************/
/* BICAP :: PWC0 :: reserved0 [31:12] */
#define BCHP_BICAP_PWC0_reserved0_MASK                             0xfffff000
#define BCHP_BICAP_PWC0_reserved0_SHIFT                            12

/* BICAP :: PWC0 :: count [11:00] */
#define BCHP_BICAP_PWC0_count_MASK                                 0x00000fff
#define BCHP_BICAP_PWC0_count_SHIFT                                0
#define BCHP_BICAP_PWC0_count_DEFAULT                              0x00000000

/***************************************************************************
 *PWC1 - BICAP PULSE WIDTH COUNTER VALUE FOR INPUT 1
 ***************************************************************************/
/* BICAP :: PWC1 :: reserved0 [31:12] */
#define BCHP_BICAP_PWC1_reserved0_MASK                             0xfffff000
#define BCHP_BICAP_PWC1_reserved0_SHIFT                            12

/* BICAP :: PWC1 :: count [11:00] */
#define BCHP_BICAP_PWC1_count_MASK                                 0x00000fff
#define BCHP_BICAP_PWC1_count_SHIFT                                0
#define BCHP_BICAP_PWC1_count_DEFAULT                              0x00000000

#endif /* #ifndef BCHP_BICAP_H__ */

/* End of File */
