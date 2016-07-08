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
 * Date:           Generated on               Thu Jun 18 10:52:58 2015
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

#ifndef BCHP_MASK_0_H__
#define BCHP_MASK_0_H__

/***************************************************************************
 *MASK_0 - Output Mask Block in Video Compositor 0/Video Intra Surface 0
 ***************************************************************************/
#define BCHP_MASK_0_REVISION_ID                  0x20645c00 /* [RO] MASK Revision register */
#define BCHP_MASK_0_CONTROL                      0x20645c04 /* [RW] MASK Control register */
#define BCHP_MASK_0_SMOOTH_LIMIT                 0x20645c08 /* [RW] Smooth Limit register */
#define BCHP_MASK_0_TEXTURE_FREQ                 0x20645c0c /* [RW] Texture Frequency register */
#define BCHP_MASK_0_RANDOM_PATTERN               0x20645c10 /* [RW] Texture Random Number Generation Pattern Control */
#define BCHP_MASK_0_SCALE_0_1_2_3                0x20645c14 /* [RW] Texture Scale 0,1,2,3 register */
#define BCHP_MASK_0_SCALE_4_5_6_7                0x20645c18 /* [RW] Texture Scale 4,5,6,7 register */
#define BCHP_MASK_0_SCRATCH_0                    0x20645c1c /* [RW] MASK Scratch register 0 */
#define BCHP_MASK_0_TEST_REG                     0x20645c20 /* [RW] MASK TEST register */

/***************************************************************************
 *REVISION_ID - MASK Revision register
 ***************************************************************************/
/* MASK_0 :: REVISION_ID :: reserved0 [31:16] */
#define BCHP_MASK_0_REVISION_ID_reserved0_MASK                     0xffff0000
#define BCHP_MASK_0_REVISION_ID_reserved0_SHIFT                    16

/* MASK_0 :: REVISION_ID :: MAJOR [15:08] */
#define BCHP_MASK_0_REVISION_ID_MAJOR_MASK                         0x0000ff00
#define BCHP_MASK_0_REVISION_ID_MAJOR_SHIFT                        8
#define BCHP_MASK_0_REVISION_ID_MAJOR_DEFAULT                      0x00000000

/* MASK_0 :: REVISION_ID :: MINOR [07:00] */
#define BCHP_MASK_0_REVISION_ID_MINOR_MASK                         0x000000ff
#define BCHP_MASK_0_REVISION_ID_MINOR_SHIFT                        0
#define BCHP_MASK_0_REVISION_ID_MINOR_DEFAULT                      0x00000028

/***************************************************************************
 *CONTROL - MASK Control register
 ***************************************************************************/
/* MASK_0 :: CONTROL :: reserved0 [31:03] */
#define BCHP_MASK_0_CONTROL_reserved0_MASK                         0xfffffff8
#define BCHP_MASK_0_CONTROL_reserved0_SHIFT                        3

/* MASK_0 :: CONTROL :: TEXTURE_ENABLE [02:02] */
#define BCHP_MASK_0_CONTROL_TEXTURE_ENABLE_MASK                    0x00000004
#define BCHP_MASK_0_CONTROL_TEXTURE_ENABLE_SHIFT                   2
#define BCHP_MASK_0_CONTROL_TEXTURE_ENABLE_DEFAULT                 0x00000000
#define BCHP_MASK_0_CONTROL_TEXTURE_ENABLE_DISABLE                 0
#define BCHP_MASK_0_CONTROL_TEXTURE_ENABLE_ENABLE                  1

/* MASK_0 :: CONTROL :: REDUCE_SMOOTH [01:01] */
#define BCHP_MASK_0_CONTROL_REDUCE_SMOOTH_MASK                     0x00000002
#define BCHP_MASK_0_CONTROL_REDUCE_SMOOTH_SHIFT                    1
#define BCHP_MASK_0_CONTROL_REDUCE_SMOOTH_DEFAULT                  0x00000000
#define BCHP_MASK_0_CONTROL_REDUCE_SMOOTH_DISABLE                  0
#define BCHP_MASK_0_CONTROL_REDUCE_SMOOTH_ENABLE                   1

/* MASK_0 :: CONTROL :: SMOOTH_ENABLE [00:00] */
#define BCHP_MASK_0_CONTROL_SMOOTH_ENABLE_MASK                     0x00000001
#define BCHP_MASK_0_CONTROL_SMOOTH_ENABLE_SHIFT                    0
#define BCHP_MASK_0_CONTROL_SMOOTH_ENABLE_DEFAULT                  0x00000000
#define BCHP_MASK_0_CONTROL_SMOOTH_ENABLE_DISABLE                  0
#define BCHP_MASK_0_CONTROL_SMOOTH_ENABLE_ENABLE                   1

/***************************************************************************
 *SMOOTH_LIMIT - Smooth Limit register
 ***************************************************************************/
/* MASK_0 :: SMOOTH_LIMIT :: reserved0 [31:04] */
#define BCHP_MASK_0_SMOOTH_LIMIT_reserved0_MASK                    0xfffffff0
#define BCHP_MASK_0_SMOOTH_LIMIT_reserved0_SHIFT                   4

/* MASK_0 :: SMOOTH_LIMIT :: VALUE [03:00] */
#define BCHP_MASK_0_SMOOTH_LIMIT_VALUE_MASK                        0x0000000f
#define BCHP_MASK_0_SMOOTH_LIMIT_VALUE_SHIFT                       0
#define BCHP_MASK_0_SMOOTH_LIMIT_VALUE_DEFAULT                     0x00000003

/***************************************************************************
 *TEXTURE_FREQ - Texture Frequency register
 ***************************************************************************/
/* MASK_0 :: TEXTURE_FREQ :: reserved0 [31:20] */
#define BCHP_MASK_0_TEXTURE_FREQ_reserved0_MASK                    0xfff00000
#define BCHP_MASK_0_TEXTURE_FREQ_reserved0_SHIFT                   20

/* MASK_0 :: TEXTURE_FREQ :: HORIZ_FREQ [19:16] */
#define BCHP_MASK_0_TEXTURE_FREQ_HORIZ_FREQ_MASK                   0x000f0000
#define BCHP_MASK_0_TEXTURE_FREQ_HORIZ_FREQ_SHIFT                  16
#define BCHP_MASK_0_TEXTURE_FREQ_HORIZ_FREQ_DEFAULT                0x00000008

/* MASK_0 :: TEXTURE_FREQ :: reserved1 [15:04] */
#define BCHP_MASK_0_TEXTURE_FREQ_reserved1_MASK                    0x0000fff0
#define BCHP_MASK_0_TEXTURE_FREQ_reserved1_SHIFT                   4

/* MASK_0 :: TEXTURE_FREQ :: VERT_FREQ [03:00] */
#define BCHP_MASK_0_TEXTURE_FREQ_VERT_FREQ_MASK                    0x0000000f
#define BCHP_MASK_0_TEXTURE_FREQ_VERT_FREQ_SHIFT                   0
#define BCHP_MASK_0_TEXTURE_FREQ_VERT_FREQ_DEFAULT                 0x00000008

/***************************************************************************
 *RANDOM_PATTERN - Texture Random Number Generation Pattern Control
 ***************************************************************************/
/* MASK_0 :: RANDOM_PATTERN :: reserved0 [31:18] */
#define BCHP_MASK_0_RANDOM_PATTERN_reserved0_MASK                  0xfffc0000
#define BCHP_MASK_0_RANDOM_PATTERN_reserved0_SHIFT                 18

/* MASK_0 :: RANDOM_PATTERN :: RNG_MODE [17:16] */
#define BCHP_MASK_0_RANDOM_PATTERN_RNG_MODE_MASK                   0x00030000
#define BCHP_MASK_0_RANDOM_PATTERN_RNG_MODE_SHIFT                  16
#define BCHP_MASK_0_RANDOM_PATTERN_RNG_MODE_DEFAULT                0x00000000
#define BCHP_MASK_0_RANDOM_PATTERN_RNG_MODE_RUN                    0
#define BCHP_MASK_0_RANDOM_PATTERN_RNG_MODE_RNG_FIELD              1
#define BCHP_MASK_0_RANDOM_PATTERN_RNG_MODE_RNG_FRAME              2

/* MASK_0 :: RANDOM_PATTERN :: RNG_SEED [15:00] */
#define BCHP_MASK_0_RANDOM_PATTERN_RNG_SEED_MASK                   0x0000ffff
#define BCHP_MASK_0_RANDOM_PATTERN_RNG_SEED_SHIFT                  0
#define BCHP_MASK_0_RANDOM_PATTERN_RNG_SEED_DEFAULT                0x00001111

/***************************************************************************
 *SCALE_0_1_2_3 - Texture Scale 0,1,2,3 register
 ***************************************************************************/
/* MASK_0 :: SCALE_0_1_2_3 :: reserved0 [31:30] */
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved0_MASK                   0xc0000000
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved0_SHIFT                  30

/* MASK_0 :: SCALE_0_1_2_3 :: MULT_0 [29:28] */
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_0_MASK                      0x30000000
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_0_SHIFT                     28
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_0_DEFAULT                   0x00000000

/* MASK_0 :: SCALE_0_1_2_3 :: reserved1 [27:27] */
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved1_MASK                   0x08000000
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved1_SHIFT                  27

/* MASK_0 :: SCALE_0_1_2_3 :: SHIFT_0 [26:24] */
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_0_MASK                     0x07000000
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_0_SHIFT                    24
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_0_DEFAULT                  0x00000004

/* MASK_0 :: SCALE_0_1_2_3 :: reserved2 [23:22] */
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved2_MASK                   0x00c00000
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved2_SHIFT                  22

/* MASK_0 :: SCALE_0_1_2_3 :: MULT_1 [21:20] */
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_1_MASK                      0x00300000
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_1_SHIFT                     20
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_1_DEFAULT                   0x00000000

/* MASK_0 :: SCALE_0_1_2_3 :: reserved3 [19:19] */
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved3_MASK                   0x00080000
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved3_SHIFT                  19

/* MASK_0 :: SCALE_0_1_2_3 :: SHIFT_1 [18:16] */
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_1_MASK                     0x00070000
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_1_SHIFT                    16
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_1_DEFAULT                  0x00000004

/* MASK_0 :: SCALE_0_1_2_3 :: reserved4 [15:14] */
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved4_MASK                   0x0000c000
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved4_SHIFT                  14

/* MASK_0 :: SCALE_0_1_2_3 :: MULT_2 [13:12] */
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_2_MASK                      0x00003000
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_2_SHIFT                     12
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_2_DEFAULT                   0x00000000

/* MASK_0 :: SCALE_0_1_2_3 :: reserved5 [11:11] */
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved5_MASK                   0x00000800
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved5_SHIFT                  11

/* MASK_0 :: SCALE_0_1_2_3 :: SHIFT_2 [10:08] */
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_2_MASK                     0x00000700
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_2_SHIFT                    8
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_2_DEFAULT                  0x00000004

/* MASK_0 :: SCALE_0_1_2_3 :: reserved6 [07:06] */
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved6_MASK                   0x000000c0
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved6_SHIFT                  6

/* MASK_0 :: SCALE_0_1_2_3 :: MULT_3 [05:04] */
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_3_MASK                      0x00000030
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_3_SHIFT                     4
#define BCHP_MASK_0_SCALE_0_1_2_3_MULT_3_DEFAULT                   0x00000000

/* MASK_0 :: SCALE_0_1_2_3 :: reserved7 [03:03] */
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved7_MASK                   0x00000008
#define BCHP_MASK_0_SCALE_0_1_2_3_reserved7_SHIFT                  3

/* MASK_0 :: SCALE_0_1_2_3 :: SHIFT_3 [02:00] */
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_3_MASK                     0x00000007
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_3_SHIFT                    0
#define BCHP_MASK_0_SCALE_0_1_2_3_SHIFT_3_DEFAULT                  0x00000004

/***************************************************************************
 *SCALE_4_5_6_7 - Texture Scale 4,5,6,7 register
 ***************************************************************************/
/* MASK_0 :: SCALE_4_5_6_7 :: reserved0 [31:30] */
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved0_MASK                   0xc0000000
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved0_SHIFT                  30

/* MASK_0 :: SCALE_4_5_6_7 :: MULT_4 [29:28] */
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_4_MASK                      0x30000000
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_4_SHIFT                     28
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_4_DEFAULT                   0x00000000

/* MASK_0 :: SCALE_4_5_6_7 :: reserved1 [27:27] */
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved1_MASK                   0x08000000
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved1_SHIFT                  27

/* MASK_0 :: SCALE_4_5_6_7 :: SHIFT_4 [26:24] */
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_4_MASK                     0x07000000
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_4_SHIFT                    24
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_4_DEFAULT                  0x00000004

/* MASK_0 :: SCALE_4_5_6_7 :: reserved2 [23:22] */
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved2_MASK                   0x00c00000
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved2_SHIFT                  22

/* MASK_0 :: SCALE_4_5_6_7 :: MULT_5 [21:20] */
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_5_MASK                      0x00300000
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_5_SHIFT                     20
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_5_DEFAULT                   0x00000000

/* MASK_0 :: SCALE_4_5_6_7 :: reserved3 [19:19] */
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved3_MASK                   0x00080000
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved3_SHIFT                  19

/* MASK_0 :: SCALE_4_5_6_7 :: SHIFT_5 [18:16] */
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_5_MASK                     0x00070000
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_5_SHIFT                    16
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_5_DEFAULT                  0x00000004

/* MASK_0 :: SCALE_4_5_6_7 :: reserved4 [15:14] */
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved4_MASK                   0x0000c000
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved4_SHIFT                  14

/* MASK_0 :: SCALE_4_5_6_7 :: MULT_6 [13:12] */
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_6_MASK                      0x00003000
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_6_SHIFT                     12
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_6_DEFAULT                   0x00000000

/* MASK_0 :: SCALE_4_5_6_7 :: reserved5 [11:11] */
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved5_MASK                   0x00000800
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved5_SHIFT                  11

/* MASK_0 :: SCALE_4_5_6_7 :: SHIFT_6 [10:08] */
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_6_MASK                     0x00000700
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_6_SHIFT                    8
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_6_DEFAULT                  0x00000004

/* MASK_0 :: SCALE_4_5_6_7 :: reserved6 [07:06] */
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved6_MASK                   0x000000c0
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved6_SHIFT                  6

/* MASK_0 :: SCALE_4_5_6_7 :: MULT_7 [05:04] */
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_7_MASK                      0x00000030
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_7_SHIFT                     4
#define BCHP_MASK_0_SCALE_4_5_6_7_MULT_7_DEFAULT                   0x00000000

/* MASK_0 :: SCALE_4_5_6_7 :: reserved7 [03:03] */
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved7_MASK                   0x00000008
#define BCHP_MASK_0_SCALE_4_5_6_7_reserved7_SHIFT                  3

/* MASK_0 :: SCALE_4_5_6_7 :: SHIFT_7 [02:00] */
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_7_MASK                     0x00000007
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_7_SHIFT                    0
#define BCHP_MASK_0_SCALE_4_5_6_7_SHIFT_7_DEFAULT                  0x00000004

/***************************************************************************
 *SCRATCH_0 - MASK Scratch register 0
 ***************************************************************************/
/* MASK_0 :: SCRATCH_0 :: VALUE [31:00] */
#define BCHP_MASK_0_SCRATCH_0_VALUE_MASK                           0xffffffff
#define BCHP_MASK_0_SCRATCH_0_VALUE_SHIFT                          0
#define BCHP_MASK_0_SCRATCH_0_VALUE_DEFAULT                        0x00000000

/***************************************************************************
 *TEST_REG - MASK TEST register
 ***************************************************************************/
/* MASK_0 :: TEST_REG :: reserved0 [31:01] */
#define BCHP_MASK_0_TEST_REG_reserved0_MASK                        0xfffffffe
#define BCHP_MASK_0_TEST_REG_reserved0_SHIFT                       1

/* MASK_0 :: TEST_REG :: UNFIX_LFSR_LD [00:00] */
#define BCHP_MASK_0_TEST_REG_UNFIX_LFSR_LD_MASK                    0x00000001
#define BCHP_MASK_0_TEST_REG_UNFIX_LFSR_LD_SHIFT                   0
#define BCHP_MASK_0_TEST_REG_UNFIX_LFSR_LD_DEFAULT                 0x00000000

#endif /* #ifndef BCHP_MASK_0_H__ */

/* End of File */
