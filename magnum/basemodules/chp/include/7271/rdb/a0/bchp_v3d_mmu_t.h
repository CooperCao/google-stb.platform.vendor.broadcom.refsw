/****************************************************************************
 *     Copyright (c) 1999-2016, Broadcom Corporation
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
 * Date:           Generated on               Fri Jan 29 14:20:26 2016
 *                 Full Compile MD5 Checksum  e5267f05b78781d897222be539adb87a
 *                     (minus title and desc)
 *                 MD5 Checksum               361d135ca9d40df0c6df868e92c12790
 *
 * lock_release:   r_287
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     697
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_V3D_MMU_T_H__
#define BCHP_V3D_MMU_T_H__

/***************************************************************************
 *V3D_MMU_T - V3D MMU Control Registers (TFU)
 ***************************************************************************/
#define BCHP_V3D_MMU_T_CTRL                      0x21201100 /* [RW] AXI_MMU Control Register */
#define BCHP_V3D_MMU_T_PT_PA_BASE                0x21201104 /* [RW] Page Table Physical Address Base */
#define BCHP_V3D_MMU_T_HITS                      0x21201108 /* [RO] TLB Hits */
#define BCHP_V3D_MMU_T_MISSES                    0x2120110c /* [RO] TLB Misses */
#define BCHP_V3D_MMU_T_STALLS                    0x21201110 /* [RO] TLB Misses */
#define BCHP_V3D_MMU_T_ADDR_CAP                  0x21201114 /* [RW] Caps the Maximum virtual page that the MMU will accept */
#define BCHP_V3D_MMU_T_SHOOT_DOWN                0x21201118 /* [RW] Shoots down specific pages from the TLB */
#define BCHP_V3D_MMU_T_BYPASS_START              0x2120111c /* [RW] Sets the Start page of the MMU Bypass */
#define BCHP_V3D_MMU_T_BYPASS_END                0x21201120 /* [RW] Sets the End page of the MMU Bypass */
#define BCHP_V3D_MMU_T_DEBUG_MISC                0x21201124 /* [RW] AXI_MMU Debug Misc */
#define BCHP_V3D_MMU_T_SECURITY                  0x21201128 /* [RW] Set individual MMU registers as secure access only */
#define BCHP_V3D_MMU_T_VIO_ID                    0x2120112c /* [RW] Record the AXI ID of the access that causes a MMU error */
#define BCHP_V3D_MMU_T_ILLEGAL_ADR               0x21201130 /* [RW] Substitute illegal PA addresses with an address that points to a dummy slave */
#define BCHP_V3D_MMU_T_VIO_ADDR                  0x21201134 /* [RW] Record the AXI ADDR of the access that causes a MMU error */

/***************************************************************************
 *CTRL - AXI_MMU Control Register
 ***************************************************************************/
/* V3D_MMU_T :: CTRL :: reserved0 [31:28] */
#define BCHP_V3D_MMU_T_CTRL_reserved0_MASK                         0xf0000000
#define BCHP_V3D_MMU_T_CTRL_reserved0_SHIFT                        28

/* V3D_MMU_T :: CTRL :: CAP_EXCEEDED [27:27] */
#define BCHP_V3D_MMU_T_CTRL_CAP_EXCEEDED_MASK                      0x08000000
#define BCHP_V3D_MMU_T_CTRL_CAP_EXCEEDED_SHIFT                     27

/* V3D_MMU_T :: CTRL :: CAP_EXCEEDED_ABORT_EN [26:26] */
#define BCHP_V3D_MMU_T_CTRL_CAP_EXCEEDED_ABORT_EN_MASK             0x04000000
#define BCHP_V3D_MMU_T_CTRL_CAP_EXCEEDED_ABORT_EN_SHIFT            26

/* V3D_MMU_T :: CTRL :: CAP_EXCEEDED_INT_EN [25:25] */
#define BCHP_V3D_MMU_T_CTRL_CAP_EXCEEDED_INT_EN_MASK               0x02000000
#define BCHP_V3D_MMU_T_CTRL_CAP_EXCEEDED_INT_EN_SHIFT              25

/* V3D_MMU_T :: CTRL :: CAP_EXCEEDED_EXCEPTION_EN [24:24] */
#define BCHP_V3D_MMU_T_CTRL_CAP_EXCEEDED_EXCEPTION_EN_MASK         0x01000000
#define BCHP_V3D_MMU_T_CTRL_CAP_EXCEEDED_EXCEPTION_EN_SHIFT        24

/* V3D_MMU_T :: CTRL :: reserved1 [23:21] */
#define BCHP_V3D_MMU_T_CTRL_reserved1_MASK                         0x00e00000
#define BCHP_V3D_MMU_T_CTRL_reserved1_SHIFT                        21

/* V3D_MMU_T :: CTRL :: PT_INVALID [20:20] */
#define BCHP_V3D_MMU_T_CTRL_PT_INVALID_MASK                        0x00100000
#define BCHP_V3D_MMU_T_CTRL_PT_INVALID_SHIFT                       20

/* V3D_MMU_T :: CTRL :: PT_INVALID_ABORT_EN [19:19] */
#define BCHP_V3D_MMU_T_CTRL_PT_INVALID_ABORT_EN_MASK               0x00080000
#define BCHP_V3D_MMU_T_CTRL_PT_INVALID_ABORT_EN_SHIFT              19

/* V3D_MMU_T :: CTRL :: PT_INVALID_INT_EN [18:18] */
#define BCHP_V3D_MMU_T_CTRL_PT_INVALID_INT_EN_MASK                 0x00040000
#define BCHP_V3D_MMU_T_CTRL_PT_INVALID_INT_EN_SHIFT                18

/* V3D_MMU_T :: CTRL :: PT_INVALID_EXCEPTION_EN [17:17] */
#define BCHP_V3D_MMU_T_CTRL_PT_INVALID_EXCEPTION_EN_MASK           0x00020000
#define BCHP_V3D_MMU_T_CTRL_PT_INVALID_EXCEPTION_EN_SHIFT          17

/* V3D_MMU_T :: CTRL :: PT_INVALID_EN [16:16] */
#define BCHP_V3D_MMU_T_CTRL_PT_INVALID_EN_MASK                     0x00010000
#define BCHP_V3D_MMU_T_CTRL_PT_INVALID_EN_SHIFT                    16

/* V3D_MMU_T :: CTRL :: reserved2 [15:13] */
#define BCHP_V3D_MMU_T_CTRL_reserved2_MASK                         0x0000e000
#define BCHP_V3D_MMU_T_CTRL_reserved2_SHIFT                        13

/* V3D_MMU_T :: CTRL :: WRITE_VIOLATION [12:12] */
#define BCHP_V3D_MMU_T_CTRL_WRITE_VIOLATION_MASK                   0x00001000
#define BCHP_V3D_MMU_T_CTRL_WRITE_VIOLATION_SHIFT                  12

/* V3D_MMU_T :: CTRL :: WRITE_VIOLATION_ABORT_EN [11:11] */
#define BCHP_V3D_MMU_T_CTRL_WRITE_VIOLATION_ABORT_EN_MASK          0x00000800
#define BCHP_V3D_MMU_T_CTRL_WRITE_VIOLATION_ABORT_EN_SHIFT         11

/* V3D_MMU_T :: CTRL :: WRITE_VIOLATION_INT_EN [10:10] */
#define BCHP_V3D_MMU_T_CTRL_WRITE_VIOLATION_INT_EN_MASK            0x00000400
#define BCHP_V3D_MMU_T_CTRL_WRITE_VIOLATION_INT_EN_SHIFT           10

/* V3D_MMU_T :: CTRL :: WRITE_VIOLATION_EXCEPTION_EN [09:09] */
#define BCHP_V3D_MMU_T_CTRL_WRITE_VIOLATION_EXCEPTION_EN_MASK      0x00000200
#define BCHP_V3D_MMU_T_CTRL_WRITE_VIOLATION_EXCEPTION_EN_SHIFT     9

/* V3D_MMU_T :: CTRL :: reserved3 [08:08] */
#define BCHP_V3D_MMU_T_CTRL_reserved3_MASK                         0x00000100
#define BCHP_V3D_MMU_T_CTRL_reserved3_SHIFT                        8

/* V3D_MMU_T :: CTRL :: TLB_CLEARING [07:07] */
#define BCHP_V3D_MMU_T_CTRL_TLB_CLEARING_MASK                      0x00000080
#define BCHP_V3D_MMU_T_CTRL_TLB_CLEARING_SHIFT                     7

/* V3D_MMU_T :: CTRL :: reserved4 [06:06] */
#define BCHP_V3D_MMU_T_CTRL_reserved4_MASK                         0x00000040
#define BCHP_V3D_MMU_T_CTRL_reserved4_SHIFT                        6

/* V3D_MMU_T :: CTRL :: spare [05:04] */
#define BCHP_V3D_MMU_T_CTRL_spare_MASK                             0x00000030
#define BCHP_V3D_MMU_T_CTRL_spare_SHIFT                            4

/* V3D_MMU_T :: CTRL :: STATS_CLEAR [03:03] */
#define BCHP_V3D_MMU_T_CTRL_STATS_CLEAR_MASK                       0x00000008
#define BCHP_V3D_MMU_T_CTRL_STATS_CLEAR_SHIFT                      3

/* V3D_MMU_T :: CTRL :: TLB_CLEAR [02:02] */
#define BCHP_V3D_MMU_T_CTRL_TLB_CLEAR_MASK                         0x00000004
#define BCHP_V3D_MMU_T_CTRL_TLB_CLEAR_SHIFT                        2

/* V3D_MMU_T :: CTRL :: STATS_ENABLE [01:01] */
#define BCHP_V3D_MMU_T_CTRL_STATS_ENABLE_MASK                      0x00000002
#define BCHP_V3D_MMU_T_CTRL_STATS_ENABLE_SHIFT                     1

/* V3D_MMU_T :: CTRL :: ENABLE [00:00] */
#define BCHP_V3D_MMU_T_CTRL_ENABLE_MASK                            0x00000001
#define BCHP_V3D_MMU_T_CTRL_ENABLE_SHIFT                           0

/***************************************************************************
 *PT_PA_BASE - Page Table Physical Address Base
 ***************************************************************************/
/* V3D_MMU_T :: PT_PA_BASE :: reserved0 [31:24] */
#define BCHP_V3D_MMU_T_PT_PA_BASE_reserved0_MASK                   0xff000000
#define BCHP_V3D_MMU_T_PT_PA_BASE_reserved0_SHIFT                  24

/* V3D_MMU_T :: PT_PA_BASE :: PAGE [23:00] */
#define BCHP_V3D_MMU_T_PT_PA_BASE_PAGE_MASK                        0x00ffffff
#define BCHP_V3D_MMU_T_PT_PA_BASE_PAGE_SHIFT                       0

/***************************************************************************
 *HITS - TLB Hits
 ***************************************************************************/
/* V3D_MMU_T :: HITS :: COUNT [31:00] */
#define BCHP_V3D_MMU_T_HITS_COUNT_MASK                             0xffffffff
#define BCHP_V3D_MMU_T_HITS_COUNT_SHIFT                            0

/***************************************************************************
 *MISSES - TLB Misses
 ***************************************************************************/
/* V3D_MMU_T :: MISSES :: COUNT [31:00] */
#define BCHP_V3D_MMU_T_MISSES_COUNT_MASK                           0xffffffff
#define BCHP_V3D_MMU_T_MISSES_COUNT_SHIFT                          0

/***************************************************************************
 *STALLS - TLB Misses
 ***************************************************************************/
/* V3D_MMU_T :: STALLS :: COUNT [31:00] */
#define BCHP_V3D_MMU_T_STALLS_COUNT_MASK                           0xffffffff
#define BCHP_V3D_MMU_T_STALLS_COUNT_SHIFT                          0

/***************************************************************************
 *ADDR_CAP - Caps the Maximum virtual page that the MMU will accept
 ***************************************************************************/
/* V3D_MMU_T :: ADDR_CAP :: ENABLE [31:31] */
#define BCHP_V3D_MMU_T_ADDR_CAP_ENABLE_MASK                        0x80000000
#define BCHP_V3D_MMU_T_ADDR_CAP_ENABLE_SHIFT                       31
#define BCHP_V3D_MMU_T_ADDR_CAP_ENABLE_DEFAULT                     0x00000000

/* V3D_MMU_T :: ADDR_CAP :: reserved0 [30:12] */
#define BCHP_V3D_MMU_T_ADDR_CAP_reserved0_MASK                     0x7ffff000
#define BCHP_V3D_MMU_T_ADDR_CAP_reserved0_SHIFT                    12

/* V3D_MMU_T :: ADDR_CAP :: MPAGE [11:00] */
#define BCHP_V3D_MMU_T_ADDR_CAP_MPAGE_MASK                         0x00000fff
#define BCHP_V3D_MMU_T_ADDR_CAP_MPAGE_SHIFT                        0
#define BCHP_V3D_MMU_T_ADDR_CAP_MPAGE_DEFAULT                      0x00000fff

/***************************************************************************
 *SHOOT_DOWN - Shoots down specific pages from the TLB
 ***************************************************************************/
/* V3D_MMU_T :: SHOOT_DOWN :: reserved0 [31:30] */
#define BCHP_V3D_MMU_T_SHOOT_DOWN_reserved0_MASK                   0xc0000000
#define BCHP_V3D_MMU_T_SHOOT_DOWN_reserved0_SHIFT                  30

/* V3D_MMU_T :: SHOOT_DOWN :: SHOOTING [29:29] */
#define BCHP_V3D_MMU_T_SHOOT_DOWN_SHOOTING_MASK                    0x20000000
#define BCHP_V3D_MMU_T_SHOOT_DOWN_SHOOTING_SHIFT                   29

/* V3D_MMU_T :: SHOOT_DOWN :: SHOOT [28:28] */
#define BCHP_V3D_MMU_T_SHOOT_DOWN_SHOOT_MASK                       0x10000000
#define BCHP_V3D_MMU_T_SHOOT_DOWN_SHOOT_SHIFT                      28

/* V3D_MMU_T :: SHOOT_DOWN :: PAGE [27:00] */
#define BCHP_V3D_MMU_T_SHOOT_DOWN_PAGE_MASK                        0x0fffffff
#define BCHP_V3D_MMU_T_SHOOT_DOWN_PAGE_SHIFT                       0

/***************************************************************************
 *BYPASS_START - Sets the Start page of the MMU Bypass
 ***************************************************************************/
/* V3D_MMU_T :: BYPASS_START :: ENABLE [31:31] */
#define BCHP_V3D_MMU_T_BYPASS_START_ENABLE_MASK                    0x80000000
#define BCHP_V3D_MMU_T_BYPASS_START_ENABLE_SHIFT                   31
#define BCHP_V3D_MMU_T_BYPASS_START_ENABLE_DEFAULT                 0x00000000

/* V3D_MMU_T :: BYPASS_START :: reserved0 [30:12] */
#define BCHP_V3D_MMU_T_BYPASS_START_reserved0_MASK                 0x7ffff000
#define BCHP_V3D_MMU_T_BYPASS_START_reserved0_SHIFT                12

/* V3D_MMU_T :: BYPASS_START :: MPAGE [11:00] */
#define BCHP_V3D_MMU_T_BYPASS_START_MPAGE_MASK                     0x00000fff
#define BCHP_V3D_MMU_T_BYPASS_START_MPAGE_SHIFT                    0
#define BCHP_V3D_MMU_T_BYPASS_START_MPAGE_DEFAULT                  0x00000000

/***************************************************************************
 *BYPASS_END - Sets the End page of the MMU Bypass
 ***************************************************************************/
/* V3D_MMU_T :: BYPASS_END :: ENABLE [31:31] */
#define BCHP_V3D_MMU_T_BYPASS_END_ENABLE_MASK                      0x80000000
#define BCHP_V3D_MMU_T_BYPASS_END_ENABLE_SHIFT                     31
#define BCHP_V3D_MMU_T_BYPASS_END_ENABLE_DEFAULT                   0x00000000

/* V3D_MMU_T :: BYPASS_END :: reserved0 [30:12] */
#define BCHP_V3D_MMU_T_BYPASS_END_reserved0_MASK                   0x7ffff000
#define BCHP_V3D_MMU_T_BYPASS_END_reserved0_SHIFT                  12

/* V3D_MMU_T :: BYPASS_END :: MPAGE [11:00] */
#define BCHP_V3D_MMU_T_BYPASS_END_MPAGE_MASK                       0x00000fff
#define BCHP_V3D_MMU_T_BYPASS_END_MPAGE_SHIFT                      0
#define BCHP_V3D_MMU_T_BYPASS_END_MPAGE_DEFAULT                    0x00000fff

/***************************************************************************
 *DEBUG_MISC - AXI_MMU Debug Misc
 ***************************************************************************/
/* V3D_MMU_T :: DEBUG_MISC :: reserved0 [31:20] */
#define BCHP_V3D_MMU_T_DEBUG_MISC_reserved0_MASK                   0xfff00000
#define BCHP_V3D_MMU_T_DEBUG_MISC_reserved0_SHIFT                  20

/* V3D_MMU_T :: DEBUG_MISC :: QOS_OVERRIDE [19:16] */
#define BCHP_V3D_MMU_T_DEBUG_MISC_QOS_OVERRIDE_MASK                0x000f0000
#define BCHP_V3D_MMU_T_DEBUG_MISC_QOS_OVERRIDE_SHIFT               16

/* V3D_MMU_T :: DEBUG_MISC :: reserved1 [15:14] */
#define BCHP_V3D_MMU_T_DEBUG_MISC_reserved1_MASK                   0x0000c000
#define BCHP_V3D_MMU_T_DEBUG_MISC_reserved1_SHIFT                  14

/* V3D_MMU_T :: DEBUG_MISC :: TLB_LIMIT [13:08] */
#define BCHP_V3D_MMU_T_DEBUG_MISC_TLB_LIMIT_MASK                   0x00003f00
#define BCHP_V3D_MMU_T_DEBUG_MISC_TLB_LIMIT_SHIFT                  8
#define BCHP_V3D_MMU_T_DEBUG_MISC_TLB_LIMIT_DEFAULT                0x0000003f

/* V3D_MMU_T :: DEBUG_MISC :: reserved2 [07:01] */
#define BCHP_V3D_MMU_T_DEBUG_MISC_reserved2_MASK                   0x000000fe
#define BCHP_V3D_MMU_T_DEBUG_MISC_reserved2_SHIFT                  1

/* V3D_MMU_T :: DEBUG_MISC :: MULTIPLE_MATCH [00:00] */
#define BCHP_V3D_MMU_T_DEBUG_MISC_MULTIPLE_MATCH_MASK              0x00000001
#define BCHP_V3D_MMU_T_DEBUG_MISC_MULTIPLE_MATCH_SHIFT             0
#define BCHP_V3D_MMU_T_DEBUG_MISC_MULTIPLE_MATCH_DEFAULT           0x00000000

/***************************************************************************
 *SECURITY - Set individual MMU registers as secure access only
 ***************************************************************************/
/* V3D_MMU_T :: SECURITY :: reserved0 [31:13] */
#define BCHP_V3D_MMU_T_SECURITY_reserved0_MASK                     0xffffe000
#define BCHP_V3D_MMU_T_SECURITY_reserved0_SHIFT                    13

/* V3D_MMU_T :: SECURITY :: enable [12:00] */
#define BCHP_V3D_MMU_T_SECURITY_enable_MASK                        0x00001fff
#define BCHP_V3D_MMU_T_SECURITY_enable_SHIFT                       0
#define BCHP_V3D_MMU_T_SECURITY_enable_DEFAULT                     0x00000000

/***************************************************************************
 *VIO_ID - Record the AXI ID of the access that causes a MMU error
 ***************************************************************************/
/* V3D_MMU_T :: VIO_ID :: reserved0 [31:16] */
#define BCHP_V3D_MMU_T_VIO_ID_reserved0_MASK                       0xffff0000
#define BCHP_V3D_MMU_T_VIO_ID_reserved0_SHIFT                      16

/* V3D_MMU_T :: VIO_ID :: enable [15:00] */
#define BCHP_V3D_MMU_T_VIO_ID_enable_MASK                          0x0000ffff
#define BCHP_V3D_MMU_T_VIO_ID_enable_SHIFT                         0
#define BCHP_V3D_MMU_T_VIO_ID_enable_DEFAULT                       0x00000000

/***************************************************************************
 *ILLEGAL_ADR - Substitute illegal PA addresses with an address that points to a dummy slave
 ***************************************************************************/
/* V3D_MMU_T :: ILLEGAL_ADR :: ENABLE [31:31] */
#define BCHP_V3D_MMU_T_ILLEGAL_ADR_ENABLE_MASK                     0x80000000
#define BCHP_V3D_MMU_T_ILLEGAL_ADR_ENABLE_SHIFT                    31
#define BCHP_V3D_MMU_T_ILLEGAL_ADR_ENABLE_DEFAULT                  0x00000000

/* V3D_MMU_T :: ILLEGAL_ADR :: PAGE [30:00] */
#define BCHP_V3D_MMU_T_ILLEGAL_ADR_PAGE_MASK                       0x7fffffff
#define BCHP_V3D_MMU_T_ILLEGAL_ADR_PAGE_SHIFT                      0
#define BCHP_V3D_MMU_T_ILLEGAL_ADR_PAGE_DEFAULT                    0x00000000

/***************************************************************************
 *VIO_ADDR - Record the AXI ADDR of the access that causes a MMU error
 ***************************************************************************/
/* V3D_MMU_T :: VIO_ADDR :: enable [31:00] */
#define BCHP_V3D_MMU_T_VIO_ADDR_enable_MASK                        0xffffffff
#define BCHP_V3D_MMU_T_VIO_ADDR_enable_SHIFT                       0
#define BCHP_V3D_MMU_T_VIO_ADDR_enable_DEFAULT                     0x00000000

#endif /* #ifndef BCHP_V3D_MMU_T_H__ */

/* End of File */
