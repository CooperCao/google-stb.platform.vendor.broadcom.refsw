/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 * The launch point for all information concerning RDB is found at:
 *   http://bcgbu.broadcom.com/RDB/SitePages/Home.aspx
 *
 * Date:           Generated on               Thu Sep 14 11:24:19 2017
 *                 Full Compile MD5 Checksum  70d290052b444707177827ab37808f44
 *                     (minus title and desc)
 *                 MD5 Checksum               210ae062c7e19ef1f4277daa8616118b
 *
 * lock_release:   r_1255
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     1749
 *                 unknown                    unknown
 *                 Perl Interpreter           5.014001
 *                 Operating System           linux
 *                 Script Source              home/tdo/public/tdo/views/refsw/git_baseline/rockford/refswsj/tools//combo_header.pl
 *                 DVTSWVER                   LOCAL home/tdo/public/tdo/views/refsw/git_baseline/rockford/refswsj/tools//combo_header.pl
 *
 *
********************************************************************************/

#ifndef BCHP_XPT_XPU_H__
#define BCHP_XPT_XPU_H__

/***************************************************************************
 *XPT_XPU - XPT XPU Control Registers
 ***************************************************************************/
#define BCHP_XPT_XPU_TESTREG                     0x20a78000 /* [RW][32] Test register - reserved */
#define BCHP_XPT_XPU_PSW                         0x20a78004 /* [RW][32] Processor status word */
#define BCHP_XPT_XPU_PSWSH                       0x20a78008 /* [RW][32] Processor status word shadow */
#define BCHP_XPT_XPU_SP                          0x20a78010 /* [RW][32] Stack pointer */
#define BCHP_XPT_XPU_PC                          0x20a78018 /* [RW][32] Program counter */
#define BCHP_XPT_XPU_STACK_0                     0x20a78020 /* [RW][32] Stack 0 */
#define BCHP_XPT_XPU_STACK_1                     0x20a78024 /* [RW][32] Stack 1 */
#define BCHP_XPT_XPU_STACK_2                     0x20a78028 /* [RW][32] Stack 2 */
#define BCHP_XPT_XPU_STACK_3                     0x20a7802c /* [RW][32] Stack 3 */
#define BCHP_XPT_XPU_REG_R0_R1                   0x20a78030 /* [RW][32] Register pair r0/r1 */
#define BCHP_XPT_XPU_REG_R2_R3                   0x20a78034 /* [RW][32] Register pair r2/r3 */
#define BCHP_XPT_XPU_REG_R4_R5                   0x20a78038 /* [RW][32] Register pair r4/r5 */
#define BCHP_XPT_XPU_REG_R6_R7                   0x20a7803c /* [RW][32] Register pair r6/r7 */
#define BCHP_XPT_XPU_REG_R8_R9                   0x20a78040 /* [RW][32] Register pair r8/r9 */
#define BCHP_XPT_XPU_REG_R10_R11                 0x20a78044 /* [RW][32] Register pair r10/r11 */
#define BCHP_XPT_XPU_REG_R12_R13                 0x20a78048 /* [RW][32] Register pair r12/r13 */
#define BCHP_XPT_XPU_REG_R14_R15                 0x20a7804c /* [RW][32] Register pair r14/r15 */

/***************************************************************************
 *TESTREG - Test register - reserved
 ***************************************************************************/
/* XPT_XPU :: TESTREG :: reserved0 [31:01] */
#define BCHP_XPT_XPU_TESTREG_reserved0_MASK                        0xfffffffe
#define BCHP_XPT_XPU_TESTREG_reserved0_SHIFT                       1

/* XPT_XPU :: TESTREG :: TESTBIT [00:00] */
#define BCHP_XPT_XPU_TESTREG_TESTBIT_MASK                          0x00000001
#define BCHP_XPT_XPU_TESTREG_TESTBIT_SHIFT                         0

/***************************************************************************
 *PSW - Processor status word
 ***************************************************************************/
/* XPT_XPU :: PSW :: reserved0 [31:11] */
#define BCHP_XPT_XPU_PSW_reserved0_MASK                            0xfffff800
#define BCHP_XPT_XPU_PSW_reserved0_SHIFT                           11

/* XPT_XPU :: PSW :: DFLAG [10:10] */
#define BCHP_XPT_XPU_PSW_DFLAG_MASK                                0x00000400
#define BCHP_XPT_XPU_PSW_DFLAG_SHIFT                               10

/* XPT_XPU :: PSW :: LFLAG [09:09] */
#define BCHP_XPT_XPU_PSW_LFLAG_MASK                                0x00000200
#define BCHP_XPT_XPU_PSW_LFLAG_SHIFT                               9

/* XPT_XPU :: PSW :: VFLAG [08:08] */
#define BCHP_XPT_XPU_PSW_VFLAG_MASK                                0x00000100
#define BCHP_XPT_XPU_PSW_VFLAG_SHIFT                               8

/* XPT_XPU :: PSW :: HFLAG [07:07] */
#define BCHP_XPT_XPU_PSW_HFLAG_MASK                                0x00000080
#define BCHP_XPT_XPU_PSW_HFLAG_SHIFT                               7

/* XPT_XPU :: PSW :: WFLAG [06:06] */
#define BCHP_XPT_XPU_PSW_WFLAG_MASK                                0x00000040
#define BCHP_XPT_XPU_PSW_WFLAG_SHIFT                               6

/* XPT_XPU :: PSW :: IFLAG [05:05] */
#define BCHP_XPT_XPU_PSW_IFLAG_MASK                                0x00000020
#define BCHP_XPT_XPU_PSW_IFLAG_SHIFT                               5

/* XPT_XPU :: PSW :: EFLAG [04:04] */
#define BCHP_XPT_XPU_PSW_EFLAG_MASK                                0x00000010
#define BCHP_XPT_XPU_PSW_EFLAG_SHIFT                               4

/* XPT_XPU :: PSW :: CFLAG [03:03] */
#define BCHP_XPT_XPU_PSW_CFLAG_MASK                                0x00000008
#define BCHP_XPT_XPU_PSW_CFLAG_SHIFT                               3

/* XPT_XPU :: PSW :: SFLAG [02:02] */
#define BCHP_XPT_XPU_PSW_SFLAG_MASK                                0x00000004
#define BCHP_XPT_XPU_PSW_SFLAG_SHIFT                               2

/* XPT_XPU :: PSW :: ZFLAG [01:01] */
#define BCHP_XPT_XPU_PSW_ZFLAG_MASK                                0x00000002
#define BCHP_XPT_XPU_PSW_ZFLAG_SHIFT                               1

/* XPT_XPU :: PSW :: OFLAG [00:00] */
#define BCHP_XPT_XPU_PSW_OFLAG_MASK                                0x00000001
#define BCHP_XPT_XPU_PSW_OFLAG_SHIFT                               0

/***************************************************************************
 *PSWSH - Processor status word shadow
 ***************************************************************************/
/* XPT_XPU :: PSWSH :: reserved0 [31:07] */
#define BCHP_XPT_XPU_PSWSH_reserved0_MASK                          0xffffff80
#define BCHP_XPT_XPU_PSWSH_reserved0_SHIFT                         7

/* XPT_XPU :: PSWSH :: WFLAG [06:06] */
#define BCHP_XPT_XPU_PSWSH_WFLAG_MASK                              0x00000040
#define BCHP_XPT_XPU_PSWSH_WFLAG_SHIFT                             6

/* XPT_XPU :: PSWSH :: reserved1 [05:04] */
#define BCHP_XPT_XPU_PSWSH_reserved1_MASK                          0x00000030
#define BCHP_XPT_XPU_PSWSH_reserved1_SHIFT                         4

/* XPT_XPU :: PSWSH :: CFLAG [03:03] */
#define BCHP_XPT_XPU_PSWSH_CFLAG_MASK                              0x00000008
#define BCHP_XPT_XPU_PSWSH_CFLAG_SHIFT                             3

/* XPT_XPU :: PSWSH :: SFLAG [02:02] */
#define BCHP_XPT_XPU_PSWSH_SFLAG_MASK                              0x00000004
#define BCHP_XPT_XPU_PSWSH_SFLAG_SHIFT                             2

/* XPT_XPU :: PSWSH :: ZFLAG [01:01] */
#define BCHP_XPT_XPU_PSWSH_ZFLAG_MASK                              0x00000002
#define BCHP_XPT_XPU_PSWSH_ZFLAG_SHIFT                             1

/* XPT_XPU :: PSWSH :: OFLAG [00:00] */
#define BCHP_XPT_XPU_PSWSH_OFLAG_MASK                              0x00000001
#define BCHP_XPT_XPU_PSWSH_OFLAG_SHIFT                             0

/***************************************************************************
 *SP - Stack pointer
 ***************************************************************************/
/* XPT_XPU :: SP :: reserved0 [31:02] */
#define BCHP_XPT_XPU_SP_reserved0_MASK                             0xfffffffc
#define BCHP_XPT_XPU_SP_reserved0_SHIFT                            2

/* XPT_XPU :: SP :: SPTR [01:00] */
#define BCHP_XPT_XPU_SP_SPTR_MASK                                  0x00000003
#define BCHP_XPT_XPU_SP_SPTR_SHIFT                                 0

/***************************************************************************
 *PC - Program counter
 ***************************************************************************/
/* XPT_XPU :: PC :: reserved0 [31:12] */
#define BCHP_XPT_XPU_PC_reserved0_MASK                             0xfffff000
#define BCHP_XPT_XPU_PC_reserved0_SHIFT                            12

/* XPT_XPU :: PC :: PC [11:00] */
#define BCHP_XPT_XPU_PC_PC_MASK                                    0x00000fff
#define BCHP_XPT_XPU_PC_PC_SHIFT                                   0

/***************************************************************************
 *STACK_0 - Stack 0
 ***************************************************************************/
/* XPT_XPU :: STACK_0 :: reserved0 [31:13] */
#define BCHP_XPT_XPU_STACK_0_reserved0_MASK                        0xffffe000
#define BCHP_XPT_XPU_STACK_0_reserved0_SHIFT                       13

/* XPT_XPU :: STACK_0 :: VALID [12:12] */
#define BCHP_XPT_XPU_STACK_0_VALID_MASK                            0x00001000
#define BCHP_XPT_XPU_STACK_0_VALID_SHIFT                           12

/* XPT_XPU :: STACK_0 :: STACK_DATA [11:00] */
#define BCHP_XPT_XPU_STACK_0_STACK_DATA_MASK                       0x00000fff
#define BCHP_XPT_XPU_STACK_0_STACK_DATA_SHIFT                      0

/***************************************************************************
 *STACK_1 - Stack 1
 ***************************************************************************/
/* XPT_XPU :: STACK_1 :: reserved0 [31:13] */
#define BCHP_XPT_XPU_STACK_1_reserved0_MASK                        0xffffe000
#define BCHP_XPT_XPU_STACK_1_reserved0_SHIFT                       13

/* XPT_XPU :: STACK_1 :: VALID [12:12] */
#define BCHP_XPT_XPU_STACK_1_VALID_MASK                            0x00001000
#define BCHP_XPT_XPU_STACK_1_VALID_SHIFT                           12

/* XPT_XPU :: STACK_1 :: STACK_DATA [11:00] */
#define BCHP_XPT_XPU_STACK_1_STACK_DATA_MASK                       0x00000fff
#define BCHP_XPT_XPU_STACK_1_STACK_DATA_SHIFT                      0

/***************************************************************************
 *STACK_2 - Stack 2
 ***************************************************************************/
/* XPT_XPU :: STACK_2 :: reserved0 [31:13] */
#define BCHP_XPT_XPU_STACK_2_reserved0_MASK                        0xffffe000
#define BCHP_XPT_XPU_STACK_2_reserved0_SHIFT                       13

/* XPT_XPU :: STACK_2 :: VALID [12:12] */
#define BCHP_XPT_XPU_STACK_2_VALID_MASK                            0x00001000
#define BCHP_XPT_XPU_STACK_2_VALID_SHIFT                           12

/* XPT_XPU :: STACK_2 :: STACK_DATA [11:00] */
#define BCHP_XPT_XPU_STACK_2_STACK_DATA_MASK                       0x00000fff
#define BCHP_XPT_XPU_STACK_2_STACK_DATA_SHIFT                      0

/***************************************************************************
 *STACK_3 - Stack 3
 ***************************************************************************/
/* XPT_XPU :: STACK_3 :: reserved0 [31:13] */
#define BCHP_XPT_XPU_STACK_3_reserved0_MASK                        0xffffe000
#define BCHP_XPT_XPU_STACK_3_reserved0_SHIFT                       13

/* XPT_XPU :: STACK_3 :: VALID [12:12] */
#define BCHP_XPT_XPU_STACK_3_VALID_MASK                            0x00001000
#define BCHP_XPT_XPU_STACK_3_VALID_SHIFT                           12

/* XPT_XPU :: STACK_3 :: STACK_DATA [11:00] */
#define BCHP_XPT_XPU_STACK_3_STACK_DATA_MASK                       0x00000fff
#define BCHP_XPT_XPU_STACK_3_STACK_DATA_SHIFT                      0

/***************************************************************************
 *REG_R0_R1 - Register pair r0/r1
 ***************************************************************************/
/* XPT_XPU :: REG_R0_R1 :: reserved0 [31:16] */
#define BCHP_XPT_XPU_REG_R0_R1_reserved0_MASK                      0xffff0000
#define BCHP_XPT_XPU_REG_R0_R1_reserved0_SHIFT                     16

/* XPT_XPU :: REG_R0_R1 :: REGISTER [15:00] */
#define BCHP_XPT_XPU_REG_R0_R1_REGISTER_MASK                       0x0000ffff
#define BCHP_XPT_XPU_REG_R0_R1_REGISTER_SHIFT                      0

/***************************************************************************
 *REG_R2_R3 - Register pair r2/r3
 ***************************************************************************/
/* XPT_XPU :: REG_R2_R3 :: reserved0 [31:16] */
#define BCHP_XPT_XPU_REG_R2_R3_reserved0_MASK                      0xffff0000
#define BCHP_XPT_XPU_REG_R2_R3_reserved0_SHIFT                     16

/* XPT_XPU :: REG_R2_R3 :: REGISTER [15:00] */
#define BCHP_XPT_XPU_REG_R2_R3_REGISTER_MASK                       0x0000ffff
#define BCHP_XPT_XPU_REG_R2_R3_REGISTER_SHIFT                      0

/***************************************************************************
 *REG_R4_R5 - Register pair r4/r5
 ***************************************************************************/
/* XPT_XPU :: REG_R4_R5 :: reserved0 [31:16] */
#define BCHP_XPT_XPU_REG_R4_R5_reserved0_MASK                      0xffff0000
#define BCHP_XPT_XPU_REG_R4_R5_reserved0_SHIFT                     16

/* XPT_XPU :: REG_R4_R5 :: REGISTER [15:00] */
#define BCHP_XPT_XPU_REG_R4_R5_REGISTER_MASK                       0x0000ffff
#define BCHP_XPT_XPU_REG_R4_R5_REGISTER_SHIFT                      0

/***************************************************************************
 *REG_R6_R7 - Register pair r6/r7
 ***************************************************************************/
/* XPT_XPU :: REG_R6_R7 :: reserved0 [31:16] */
#define BCHP_XPT_XPU_REG_R6_R7_reserved0_MASK                      0xffff0000
#define BCHP_XPT_XPU_REG_R6_R7_reserved0_SHIFT                     16

/* XPT_XPU :: REG_R6_R7 :: REGISTER [15:00] */
#define BCHP_XPT_XPU_REG_R6_R7_REGISTER_MASK                       0x0000ffff
#define BCHP_XPT_XPU_REG_R6_R7_REGISTER_SHIFT                      0

/***************************************************************************
 *REG_R8_R9 - Register pair r8/r9
 ***************************************************************************/
/* XPT_XPU :: REG_R8_R9 :: reserved0 [31:16] */
#define BCHP_XPT_XPU_REG_R8_R9_reserved0_MASK                      0xffff0000
#define BCHP_XPT_XPU_REG_R8_R9_reserved0_SHIFT                     16

/* XPT_XPU :: REG_R8_R9 :: REGISTER [15:00] */
#define BCHP_XPT_XPU_REG_R8_R9_REGISTER_MASK                       0x0000ffff
#define BCHP_XPT_XPU_REG_R8_R9_REGISTER_SHIFT                      0

/***************************************************************************
 *REG_R10_R11 - Register pair r10/r11
 ***************************************************************************/
/* XPT_XPU :: REG_R10_R11 :: reserved0 [31:16] */
#define BCHP_XPT_XPU_REG_R10_R11_reserved0_MASK                    0xffff0000
#define BCHP_XPT_XPU_REG_R10_R11_reserved0_SHIFT                   16

/* XPT_XPU :: REG_R10_R11 :: REGISTER [15:00] */
#define BCHP_XPT_XPU_REG_R10_R11_REGISTER_MASK                     0x0000ffff
#define BCHP_XPT_XPU_REG_R10_R11_REGISTER_SHIFT                    0

/***************************************************************************
 *REG_R12_R13 - Register pair r12/r13
 ***************************************************************************/
/* XPT_XPU :: REG_R12_R13 :: reserved0 [31:16] */
#define BCHP_XPT_XPU_REG_R12_R13_reserved0_MASK                    0xffff0000
#define BCHP_XPT_XPU_REG_R12_R13_reserved0_SHIFT                   16

/* XPT_XPU :: REG_R12_R13 :: REGISTER [15:00] */
#define BCHP_XPT_XPU_REG_R12_R13_REGISTER_MASK                     0x0000ffff
#define BCHP_XPT_XPU_REG_R12_R13_REGISTER_SHIFT                    0

/***************************************************************************
 *REG_R14_R15 - Register pair r14/r15
 ***************************************************************************/
/* XPT_XPU :: REG_R14_R15 :: reserved0 [31:16] */
#define BCHP_XPT_XPU_REG_R14_R15_reserved0_MASK                    0xffff0000
#define BCHP_XPT_XPU_REG_R14_R15_reserved0_SHIFT                   16

/* XPT_XPU :: REG_R14_R15 :: REGISTER [15:00] */
#define BCHP_XPT_XPU_REG_R14_R15_REGISTER_MASK                     0x0000ffff
#define BCHP_XPT_XPU_REG_R14_R15_REGISTER_SHIFT                    0

/***************************************************************************
 *IMEM%i - Instruction memory address 0..4095
 ***************************************************************************/
#define BCHP_XPT_XPU_IMEMi_ARRAY_BASE                              0x20a78800
#define BCHP_XPT_XPU_IMEMi_ARRAY_START                             0
#define BCHP_XPT_XPU_IMEMi_ARRAY_END                               4095
#define BCHP_XPT_XPU_IMEMi_ARRAY_ELEMENT_SIZE                      32

/***************************************************************************
 *IMEM%i - Instruction memory address 0..4095
 ***************************************************************************/
/* XPT_XPU :: IMEMi :: reserved0 [31:22] */
#define BCHP_XPT_XPU_IMEMi_reserved0_MASK                          0xffc00000
#define BCHP_XPT_XPU_IMEMi_reserved0_SHIFT                         22

/* XPT_XPU :: IMEMi :: INSTRUCTION [21:00] */
#define BCHP_XPT_XPU_IMEMi_INSTRUCTION_MASK                        0x003fffff
#define BCHP_XPT_XPU_IMEMi_INSTRUCTION_SHIFT                       0


#endif /* #ifndef BCHP_XPT_XPU_H__ */

/* End of File */
