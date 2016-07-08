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
 * Date:           Generated on               Mon Aug 24 11:29:34 2015
 *                 Full Compile MD5 Checksum  cecd4eac458fcdc4b77c82d0630f17be
 *                     (minus title and desc)
 *                 MD5 Checksum               c9a18191e1cdbfad4487ef21d91e95fc
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     126
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_IT_1_H__
#define BCHP_IT_1_H__

/***************************************************************************
 *IT_1 - Input and Timing Control IT_1
 ***************************************************************************/
#define BCHP_IT_1_IT_REV_ID                      0x206e2000 /* [RO] Revision ID register */
#define BCHP_IT_1_TG_CONFIG                      0x206e2004 /* [RW] Timing Generator Configuration Register */
#define BCHP_IT_1_ADDR_0_3                       0x206e2008 /* [RW] Timing Generator Address 0-3 Register */
#define BCHP_IT_1_ADDR_4_6                       0x206e200c /* [RW] Timing Generator Address 4-6 Register */
#define BCHP_IT_1_STACK_reg_0_1                  0x206e2010 /* [RW] General Lookup Registers 0&1 Register */
#define BCHP_IT_1_STACK_reg_2_3                  0x206e2014 /* [RW] General Lookup Registers 2&3 Register */
#define BCHP_IT_1_STACK_reg_4_5                  0x206e2018 /* [RW] General Lookup Registers 4&5 Register */
#define BCHP_IT_1_STACK_reg_6_7                  0x206e201c /* [RW] General Lookup Registers 6&7 Register */
#define BCHP_IT_1_EVENT_SELECTION                0x206e2020 /* [RW] Timing Generator Event Selection Register. */
#define BCHP_IT_1_PCL_0                          0x206e2024 /* [RW] External Digital H/V Syncs and Negative Sync PCL Register. */
#define BCHP_IT_1_PCL_1                          0x206e2028 /* [RW] Color Burst and Bottles PCL Register. */
#define BCHP_IT_1_PCL_2                          0x206e202c /* [RW] External Digital H/V Syncs and U and V flip PCL Register. */
#define BCHP_IT_1_PCL_3                          0x206e2030 /* [RW] VSync, VBlank, Active Video and Odd/Even Field PCL Register. */
#define BCHP_IT_1_PCL_4                          0x206e2034 /* [RW] Positive Sync A/B PCL Register. */
#define BCHP_IT_1_PCL_5                          0x206e2038 /* [RW] Patten Generation PCL Register. */
#define BCHP_IT_1_PCL_6                          0x206e203c /* [RW] Positive Sync C/D PCL Register. */
#define BCHP_IT_1_PCL_7                          0x206e2040 /* [RW] Negative Sync D1 PCL Register. */
#define BCHP_IT_1_PCL_8                          0x206e2044 /* [RW] Negative Sync D2 PCL Register. */
#define BCHP_IT_1_STACK_reg_8_9                  0x206e2048 /* [RW] General Lookup Registers 8&9 Register */
#define BCHP_IT_1_BVB_SIZE                       0x206e204c /* [RW] BVB Size Register. */
#define BCHP_IT_1_BVB_RSTATUS                    0x206e2050 /* [RO] BVB status read Register. */
#define BCHP_IT_1_BVB_CSTATUS                    0x206e2054 /* [WO] BVB status clear Register. */
#define BCHP_IT_1_VEC_TRIGGER_0                  0x206e2058 /* [RW] VEC Trigger Register 0 */
#define BCHP_IT_1_VEC_TRIGGER_1                  0x206e205c /* [RW] VEC Trigger Register 1 */
#define BCHP_IT_1_VEC_CTRL_STAT                  0x206e2060 /* [RO] VEC Control BUS Status Register */
#define BCHP_IT_1_IT_LCNTR                       0x206e2064 /* [RO] Line Counter Register */
#define BCHP_IT_1_ALT_RM_CNTRL                   0x206e2068 /* [RW] ALTERNATE RATE MANAGER CONTROL REGISTER */
#define BCHP_IT_1_CABLE_DETECT_SEL               0x206e206c /* [RW] Cable detect select register */
#define BCHP_IT_1_ACCESS_CNTRL                   0x206e2070 /* [WO] VEC Access Control Register */
#define BCHP_IT_1_ACCESS_STATUS                  0x206e2074 /* [RO] VEC Access Status Register */
#define BCHP_IT_1_MSYNC_CTRL                     0x206e21bc /* [RW] Master Sync Control */
#define BCHP_IT_1_SSYNC_CTRL                     0x206e21c0 /* [RW] Slave Sync Control */
#define BCHP_IT_1_MS_TIMEOUT                     0x206e21c4 /* [RW] Master Slave Time Out register */
#define BCHP_IT_1_MSSYNC_START                   0x206e21c8 /* [RW] Master Slave Sync Start */
#define BCHP_IT_1_MSSYNC_PCL                     0x206e21cc /* [RW] Master Slave flag select PCL */
#define BCHP_IT_1_MSYNC_PHASE                    0x206e21d0 /* [RW] Master Sync Phase */
#define BCHP_IT_1_EOF0_LINE                      0x206e21d4 /* [RW] Field0 End line number for interlaced format */
#define BCHP_IT_1_MSSYNC_STATUS                  0x206e21d8 /* [RO] "Status register for MSSYNC" */
#define BCHP_IT_1_AS_CONTROL                     0x206e22b4 /* [RW] Active Space Control register */
#define BCHP_IT_1_AS_PIXEL_C0_C1                 0x206e22b8 /* [RW] Active Space Pixel Value Register */
#define BCHP_IT_1_AS_PIXEL_C2                    0x206e22bc /* [RW] Active Space Pixel Value Register */
#define BCHP_IT_1_AS_LINE_NUMBER                 0x206e22c0 /* [RW] Active Space Line Number Register */

/***************************************************************************
 *MC_ADDR_%i - CALLI Immediate registers
 ***************************************************************************/
#define BCHP_IT_1_MC_ADDR_i_ARRAY_BASE                             0x206e2078
#define BCHP_IT_1_MC_ADDR_i_ARRAY_START                            0
#define BCHP_IT_1_MC_ADDR_i_ARRAY_END                              15
#define BCHP_IT_1_MC_ADDR_i_ARRAY_ELEMENT_SIZE                     32

/***************************************************************************
 *MC_ADDR_%i - CALLI Immediate registers
 ***************************************************************************/
/* IT_1 :: MC_ADDR_i :: reserved0 [31:24] */
#define BCHP_IT_1_MC_ADDR_i_reserved0_MASK                         0xff000000
#define BCHP_IT_1_MC_ADDR_i_reserved0_SHIFT                        24

/* IT_1 :: MC_ADDR_i :: ADDR [23:12] */
#define BCHP_IT_1_MC_ADDR_i_ADDR_MASK                              0x00fff000
#define BCHP_IT_1_MC_ADDR_i_ADDR_SHIFT                             12
#define BCHP_IT_1_MC_ADDR_i_ADDR_DEFAULT                           0x00000000

/* IT_1 :: MC_ADDR_i :: COUNT [11:00] */
#define BCHP_IT_1_MC_ADDR_i_COUNT_MASK                             0x00000fff
#define BCHP_IT_1_MC_ADDR_i_COUNT_SHIFT                            0
#define BCHP_IT_1_MC_ADDR_i_COUNT_DEFAULT                          0x00000000


/***************************************************************************
 *SSYNC_LINE%i - Slave Sync Phase Lines
 ***************************************************************************/
#define BCHP_IT_1_SSYNC_LINEi_ARRAY_BASE                           0x206e21e0
#define BCHP_IT_1_SSYNC_LINEi_ARRAY_START                          0
#define BCHP_IT_1_SSYNC_LINEi_ARRAY_END                            15
#define BCHP_IT_1_SSYNC_LINEi_ARRAY_ELEMENT_SIZE                   32

/***************************************************************************
 *SSYNC_LINE%i - Slave Sync Phase Lines
 ***************************************************************************/
/* IT_1 :: SSYNC_LINEi :: reserved0 [31:11] */
#define BCHP_IT_1_SSYNC_LINEi_reserved0_MASK                       0xfffff800
#define BCHP_IT_1_SSYNC_LINEi_reserved0_SHIFT                      11

/* IT_1 :: SSYNC_LINEi :: COUNT [10:00] */
#define BCHP_IT_1_SSYNC_LINEi_COUNT_MASK                           0x000007ff
#define BCHP_IT_1_SSYNC_LINEi_COUNT_SHIFT                          0
#define BCHP_IT_1_SSYNC_LINEi_COUNT_DEFAULT                        0x00000000


/***************************************************************************
 *SSYNC_PHASE%i - Slave Sync Phases
 ***************************************************************************/
#define BCHP_IT_1_SSYNC_PHASEi_ARRAY_BASE                          0x206e2220
#define BCHP_IT_1_SSYNC_PHASEi_ARRAY_START                         0
#define BCHP_IT_1_SSYNC_PHASEi_ARRAY_END                           15
#define BCHP_IT_1_SSYNC_PHASEi_ARRAY_ELEMENT_SIZE                  32

/***************************************************************************
 *SSYNC_PHASE%i - Slave Sync Phases
 ***************************************************************************/
/* IT_1 :: SSYNC_PHASEi :: VALUE [31:00] */
#define BCHP_IT_1_SSYNC_PHASEi_VALUE_MASK                          0xffffffff
#define BCHP_IT_1_SSYNC_PHASEi_VALUE_SHIFT                         0
#define BCHP_IT_1_SSYNC_PHASEi_VALUE_DEFAULT                       0x00000000


/***************************************************************************
 *MICRO_INSTRUCTION%i - Timing Generator RAM Register at location 0..255
 ***************************************************************************/
#define BCHP_IT_1_MICRO_INSTRUCTIONi_ARRAY_BASE                    0x206e2400
#define BCHP_IT_1_MICRO_INSTRUCTIONi_ARRAY_START                   0
#define BCHP_IT_1_MICRO_INSTRUCTIONi_ARRAY_END                     255
#define BCHP_IT_1_MICRO_INSTRUCTIONi_ARRAY_ELEMENT_SIZE            32

/***************************************************************************
 *MICRO_INSTRUCTION%i - Timing Generator RAM Register at location 0..255
 ***************************************************************************/
/* IT_1 :: MICRO_INSTRUCTIONi :: reserved0 [31:24] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_reserved0_MASK                0xff000000
#define BCHP_IT_1_MICRO_INSTRUCTIONi_reserved0_SHIFT               24

/* IT_1 :: MICRO_INSTRUCTIONi :: OPCODE [23:21] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_OPCODE_MASK                   0x00e00000
#define BCHP_IT_1_MICRO_INSTRUCTIONi_OPCODE_SHIFT                  21
#define BCHP_IT_1_MICRO_INSTRUCTIONi_OPCODE_NOP                    0
#define BCHP_IT_1_MICRO_INSTRUCTIONi_OPCODE_SCOUNT                 1
#define BCHP_IT_1_MICRO_INSTRUCTIONi_OPCODE_ECOUNT                 2
#define BCHP_IT_1_MICRO_INSTRUCTIONi_OPCODE_CALL                   3
#define BCHP_IT_1_MICRO_INSTRUCTIONi_OPCODE_JUMP                   4
#define BCHP_IT_1_MICRO_INSTRUCTIONi_OPCODE_RELOAD                 5
#define BCHP_IT_1_MICRO_INSTRUCTIONi_OPCODE_LOAD                   6
#define BCHP_IT_1_MICRO_INSTRUCTIONi_OPCODE_CALLI                  7

/* IT_1 :: MICRO_INSTRUCTIONi :: RETURN_FLAG [20:20] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_RETURN_FLAG_MASK              0x00100000
#define BCHP_IT_1_MICRO_INSTRUCTIONi_RETURN_FLAG_SHIFT             20

/* IT_1 :: MICRO_INSTRUCTIONi :: FLAGS_OR_ADDR [19:12] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_FLAGS_OR_ADDR_MASK            0x000ff000
#define BCHP_IT_1_MICRO_INSTRUCTIONi_FLAGS_OR_ADDR_SHIFT           12

/* union - case ECOUNT [11:00] */
/* IT_1 :: MICRO_INSTRUCTIONi :: ECOUNT :: EVENT_EDGE [11:11] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_ECOUNT_EVENT_EDGE_MASK        0x00000800
#define BCHP_IT_1_MICRO_INSTRUCTIONi_ECOUNT_EVENT_EDGE_SHIFT       11

/* IT_1 :: MICRO_INSTRUCTIONi :: ECOUNT :: COUNT [10:00] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_ECOUNT_COUNT_MASK             0x000007ff
#define BCHP_IT_1_MICRO_INSTRUCTIONi_ECOUNT_COUNT_SHIFT            0

/* union - case SCOUNT [11:00] */
/* IT_1 :: MICRO_INSTRUCTIONi :: SCOUNT :: COUNT [11:00] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_SCOUNT_COUNT_MASK             0x00000fff
#define BCHP_IT_1_MICRO_INSTRUCTIONi_SCOUNT_COUNT_SHIFT            0

/* union - case CALL [11:00] */
/* IT_1 :: MICRO_INSTRUCTIONi :: CALL :: COUNT [11:00] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_CALL_COUNT_MASK               0x00000fff
#define BCHP_IT_1_MICRO_INSTRUCTIONi_CALL_COUNT_SHIFT              0

/* union - case JUMP [11:00] */
/* IT_1 :: MICRO_INSTRUCTIONi :: JUMP :: reserved0 [11:10] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_JUMP_reserved0_MASK           0x00000c00
#define BCHP_IT_1_MICRO_INSTRUCTIONi_JUMP_reserved0_SHIFT          10

/* IT_1 :: MICRO_INSTRUCTIONi :: JUMP :: STACK_SELECT [09:00] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_JUMP_STACK_SELECT_MASK        0x000003ff
#define BCHP_IT_1_MICRO_INSTRUCTIONi_JUMP_STACK_SELECT_SHIFT       0

/* union - case LOAD [11:00] */
/* IT_1 :: MICRO_INSTRUCTIONi :: LOAD :: reserved0 [11:10] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_LOAD_reserved0_MASK           0x00000c00
#define BCHP_IT_1_MICRO_INSTRUCTIONi_LOAD_reserved0_SHIFT          10

/* IT_1 :: MICRO_INSTRUCTIONi :: LOAD :: STACK_SELECT [09:00] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_LOAD_STACK_SELECT_MASK        0x000003ff
#define BCHP_IT_1_MICRO_INSTRUCTIONi_LOAD_STACK_SELECT_SHIFT       0

/* union - case RELOAD [11:00] */
/* IT_1 :: MICRO_INSTRUCTIONi :: RELOAD :: reserved0 [11:10] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_RELOAD_reserved0_MASK         0x00000c00
#define BCHP_IT_1_MICRO_INSTRUCTIONi_RELOAD_reserved0_SHIFT        10

/* IT_1 :: MICRO_INSTRUCTIONi :: RELOAD :: STACK_SELECT [09:00] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_RELOAD_STACK_SELECT_MASK      0x000003ff
#define BCHP_IT_1_MICRO_INSTRUCTIONi_RELOAD_STACK_SELECT_SHIFT     0

/* union - case default [11:00] */
/* IT_1 :: MICRO_INSTRUCTIONi :: default :: reserved0 [11:00] */
#define BCHP_IT_1_MICRO_INSTRUCTIONi_default_reserved0_MASK        0x00000fff
#define BCHP_IT_1_MICRO_INSTRUCTIONi_default_reserved0_SHIFT       0


#endif /* #ifndef BCHP_IT_1_H__ */

/* End of File */
