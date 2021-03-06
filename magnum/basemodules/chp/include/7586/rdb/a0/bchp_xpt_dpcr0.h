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
 * Date:           Generated on               Wed Feb 11 10:13:57 2015
 *                 Full Compile MD5 Checksum  f7f4bd55341805fcfe958ba5e47e65f4
 *                     (minus title and desc)
 *                 MD5 Checksum               95b679a9655597a92593cae55222c397
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15653
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_XPT_DPCR0_H__
#define BCHP_XPT_DPCR0_H__

/***************************************************************************
 *XPT_DPCR0 - XPT DPCR0 Control Registers
 ***************************************************************************/
#define BCHP_XPT_DPCR0_PID_CH                    0x00a02000 /* [RW] Data Transport PCR PID Channel Register */
#define BCHP_XPT_DPCR0_CTRL                      0x00a02004 /* [RW] Data Transport PCR Control Register */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG           0x00a02008 /* [RW] Interrupt Status Register */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_EN        0x00a0200c /* [RW] Interrupt Status Enable Register */
#define BCHP_XPT_DPCR0_STC_EXT_CTRL              0x00a02010 /* [RW] Data Transport PCR STC Extension Control Register */
#define BCHP_XPT_DPCR0_MAX_PCR_ERROR             0x00a02020 /* [RW] Data Transport PCR Max PCR Error Register */
#define BCHP_XPT_DPCR0_SEND_BASE                 0x00a02024 /* [RW] Data Transport PCR Send Base Register */
#define BCHP_XPT_DPCR0_SEND_EXT                  0x00a02028 /* [RW] Data Transport PCR Send Extension Register */
#define BCHP_XPT_DPCR0_STC_EXT_CTRL27            0x00a0202c /* [RO] Data Transport PCR STC Extension Control Register (Test Only) */
#define BCHP_XPT_DPCR0_STC_HI                    0x00a02030 /* [RO] Data Transport PCR STC MSBs Register */
#define BCHP_XPT_DPCR0_STC_LO                    0x00a02034 /* [RO] Data Transport PCR STC LSBs Register */
#define BCHP_XPT_DPCR0_PWM_CTRLVALUE             0x00a02038 /* [RO] Data Transport PCR PWM Control Value Register */
#define BCHP_XPT_DPCR0_LAST_PCR_HI               0x00a0203c /* [RO] Data Transport PCR Last PCR MSBs Register */
#define BCHP_XPT_DPCR0_LAST_PCR_LO               0x00a02040 /* [RO] Data Transport PCR Last PCR LSBs Register */
#define BCHP_XPT_DPCR0_STC_BASE_LSBS             0x00a02048 /* [RO] Data Transport PCR STC Base LSBs Register */
#define BCHP_XPT_DPCR0_PHASE_ERROR               0x00a0204c /* [RO] Timebase Last Phase Error */
#define BCHP_XPT_DPCR0_LOOP_CTRL                 0x00a02050 /* [RW] Timebase Control */
#define BCHP_XPT_DPCR0_REF_PCR_PRESCALE          0x00a02054 /* [RW] Timebase Frequency Reference Prescale Control */
#define BCHP_XPT_DPCR0_REF_PCR_INC               0x00a02058 /* [RW] Timebase Frequency Reference Increment Control */
#define BCHP_XPT_DPCR0_CENTER                    0x00a0205c /* [RW] Timebase Center Frequency */
#define BCHP_XPT_DPCR0_ACCUM_VALUE               0x00a02060 /* [RW] Timebase Loop Filter Integrator */
#define BCHP_XPT_DPCR0_PCR_COUNT                 0x00a02064 /* [RO] Data Transport PCR Phase Error Register */
#define BCHP_XPT_DPCR0_SOFT_PCR_CTRL             0x00a02068 /* [RW] Data Transport Soft PCR Control Register */
#define BCHP_XPT_DPCR0_SOFT_PCR_BASE             0x00a0206c /* [RW] Data Transport Soft PCR BASE Register */
#define BCHP_XPT_DPCR0_SOFT_PCR_EXT              0x00a02070 /* [RW] Data Transport Soft PCR Extension Register */
#define BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP         0x00a02074 /* [RW] Timebase Phase Error Control */
#define BCHP_XPT_DPCR0_TIMEBASE_INPUT_SEL        0x00a02078 /* [RW] Timebase Input Select for Timebase Loop */

/***************************************************************************
 *PID_CH - Data Transport PCR PID Channel Register
 ***************************************************************************/
/* XPT_DPCR0 :: PID_CH :: reserved0 [31:13] */
#define BCHP_XPT_DPCR0_PID_CH_reserved0_MASK                       0xffffe000
#define BCHP_XPT_DPCR0_PID_CH_reserved0_SHIFT                      13

/* XPT_DPCR0 :: PID_CH :: PCR_PID_CH_VALID [12:12] */
#define BCHP_XPT_DPCR0_PID_CH_PCR_PID_CH_VALID_MASK                0x00001000
#define BCHP_XPT_DPCR0_PID_CH_PCR_PID_CH_VALID_SHIFT               12
#define BCHP_XPT_DPCR0_PID_CH_PCR_PID_CH_VALID_DEFAULT             0x00000000

/* XPT_DPCR0 :: PID_CH :: PCR_PID_CH [11:00] */
#define BCHP_XPT_DPCR0_PID_CH_PCR_PID_CH_MASK                      0x00000fff
#define BCHP_XPT_DPCR0_PID_CH_PCR_PID_CH_SHIFT                     0
#define BCHP_XPT_DPCR0_PID_CH_PCR_PID_CH_DEFAULT                   0x00000000

/***************************************************************************
 *CTRL - Data Transport PCR Control Register
 ***************************************************************************/
/* XPT_DPCR0 :: CTRL :: reserved0 [31:23] */
#define BCHP_XPT_DPCR0_CTRL_reserved0_MASK                         0xff800000
#define BCHP_XPT_DPCR0_CTRL_reserved0_SHIFT                        23

/* XPT_DPCR0 :: CTRL :: PCR_BUSY [22:22] */
#define BCHP_XPT_DPCR0_CTRL_PCR_BUSY_MASK                          0x00400000
#define BCHP_XPT_DPCR0_CTRL_PCR_BUSY_SHIFT                         22
#define BCHP_XPT_DPCR0_CTRL_PCR_BUSY_DEFAULT                       0x00000000

/* XPT_DPCR0 :: CTRL :: PCR_STC_LOAD_CTRL [21:21] */
#define BCHP_XPT_DPCR0_CTRL_PCR_STC_LOAD_CTRL_MASK                 0x00200000
#define BCHP_XPT_DPCR0_CTRL_PCR_STC_LOAD_CTRL_SHIFT                21
#define BCHP_XPT_DPCR0_CTRL_PCR_STC_LOAD_CTRL_DEFAULT              0x00000000

/* XPT_DPCR0 :: CTRL :: PCR_TWO_ERR_REACQUIRE_EN [20:20] */
#define BCHP_XPT_DPCR0_CTRL_PCR_TWO_ERR_REACQUIRE_EN_MASK          0x00100000
#define BCHP_XPT_DPCR0_CTRL_PCR_TWO_ERR_REACQUIRE_EN_SHIFT         20
#define BCHP_XPT_DPCR0_CTRL_PCR_TWO_ERR_REACQUIRE_EN_DEFAULT       0x00000000

/* XPT_DPCR0 :: CTRL :: PCR_ONE_ERR_REACQUIRE_EN [19:19] */
#define BCHP_XPT_DPCR0_CTRL_PCR_ONE_ERR_REACQUIRE_EN_MASK          0x00080000
#define BCHP_XPT_DPCR0_CTRL_PCR_ONE_ERR_REACQUIRE_EN_SHIFT         19
#define BCHP_XPT_DPCR0_CTRL_PCR_ONE_ERR_REACQUIRE_EN_DEFAULT       0x00000000

/* XPT_DPCR0 :: CTRL :: reserved1 [18:15] */
#define BCHP_XPT_DPCR0_CTRL_reserved1_MASK                         0x00078000
#define BCHP_XPT_DPCR0_CTRL_reserved1_SHIFT                        15

/* XPT_DPCR0 :: CTRL :: PCR_PACKET_MODE [14:14] */
#define BCHP_XPT_DPCR0_CTRL_PCR_PACKET_MODE_MASK                   0x00004000
#define BCHP_XPT_DPCR0_CTRL_PCR_PACKET_MODE_SHIFT                  14
#define BCHP_XPT_DPCR0_CTRL_PCR_PACKET_MODE_DEFAULT                0x00000000

/* XPT_DPCR0 :: CTRL :: PCR_SOFT_INIT [13:13] */
#define BCHP_XPT_DPCR0_CTRL_PCR_SOFT_INIT_MASK                     0x00002000
#define BCHP_XPT_DPCR0_CTRL_PCR_SOFT_INIT_SHIFT                    13
#define BCHP_XPT_DPCR0_CTRL_PCR_SOFT_INIT_DEFAULT                  0x00000000

/* XPT_DPCR0 :: CTRL :: CLR_INTEGRATOR_COUNT_CTRL [12:12] */
#define BCHP_XPT_DPCR0_CTRL_CLR_INTEGRATOR_COUNT_CTRL_MASK         0x00001000
#define BCHP_XPT_DPCR0_CTRL_CLR_INTEGRATOR_COUNT_CTRL_SHIFT        12
#define BCHP_XPT_DPCR0_CTRL_CLR_INTEGRATOR_COUNT_CTRL_DEFAULT      0x00000000

/* XPT_DPCR0 :: CTRL :: reserved2 [11:07] */
#define BCHP_XPT_DPCR0_CTRL_reserved2_MASK                         0x00000f80
#define BCHP_XPT_DPCR0_CTRL_reserved2_SHIFT                        7

/* XPT_DPCR0 :: CTRL :: ERROR_INT_TEST_MODE [06:06] */
#define BCHP_XPT_DPCR0_CTRL_ERROR_INT_TEST_MODE_MASK               0x00000040
#define BCHP_XPT_DPCR0_CTRL_ERROR_INT_TEST_MODE_SHIFT              6
#define BCHP_XPT_DPCR0_CTRL_ERROR_INT_TEST_MODE_DEFAULT            0x00000000

/* XPT_DPCR0 :: CTRL :: reserved3 [05:00] */
#define BCHP_XPT_DPCR0_CTRL_reserved3_MASK                         0x0000003f
#define BCHP_XPT_DPCR0_CTRL_reserved3_SHIFT                        0

/***************************************************************************
 *INTR_STATUS_REG - Interrupt Status Register
 ***************************************************************************/
/* XPT_DPCR0 :: INTR_STATUS_REG :: reserved0 [31:07] */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_reserved0_MASK              0xffffff80
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_reserved0_SHIFT             7

/* XPT_DPCR0 :: INTR_STATUS_REG :: TWO_PCR_ERROR [06:06] */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_TWO_PCR_ERROR_MASK          0x00000040
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_TWO_PCR_ERROR_SHIFT         6
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_TWO_PCR_ERROR_DEFAULT       0x00000000

/* XPT_DPCR0 :: INTR_STATUS_REG :: ONE_PCR_ERROR [05:05] */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_ONE_PCR_ERROR_MASK          0x00000020
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_ONE_PCR_ERROR_SHIFT         5
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_ONE_PCR_ERROR_DEFAULT       0x00000000

/* XPT_DPCR0 :: INTR_STATUS_REG :: PHASE_SAT_INT [04:04] */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_PHASE_SAT_INT_MASK          0x00000010
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_PHASE_SAT_INT_SHIFT         4
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_PHASE_SAT_INT_DEFAULT       0x00000000

/* XPT_DPCR0 :: INTR_STATUS_REG :: PHASE_CMP_INT [03:03] */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_PHASE_CMP_INT_MASK          0x00000008
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_PHASE_CMP_INT_SHIFT         3
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_PHASE_CMP_INT_DEFAULT       0x00000000

/* XPT_DPCR0 :: INTR_STATUS_REG :: DISCONTINUITY_INT [02:02] */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_DISCONTINUITY_INT_MASK      0x00000004
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_DISCONTINUITY_INT_SHIFT     2
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_DISCONTINUITY_INT_DEFAULT   0x00000000

/* XPT_DPCR0 :: INTR_STATUS_REG :: LOAD_INT [01:01] */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_LOAD_INT_MASK               0x00000002
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_LOAD_INT_SHIFT              1
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_LOAD_INT_DEFAULT            0x00000000

/* XPT_DPCR0 :: INTR_STATUS_REG :: PCR_CNT_WRAP [00:00] */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_PCR_CNT_WRAP_MASK           0x00000001
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_PCR_CNT_WRAP_SHIFT          0
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_PCR_CNT_WRAP_DEFAULT        0x00000000

/***************************************************************************
 *INTR_STATUS_REG_EN - Interrupt Status Enable Register
 ***************************************************************************/
/* XPT_DPCR0 :: INTR_STATUS_REG_EN :: reserved0 [31:07] */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_EN_reserved0_MASK           0xffffff80
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_EN_reserved0_SHIFT          7

/* XPT_DPCR0 :: INTR_STATUS_REG_EN :: INTR_STATUS_REG_EN [06:00] */
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_EN_INTR_STATUS_REG_EN_MASK  0x0000007f
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_EN_INTR_STATUS_REG_EN_SHIFT 0
#define BCHP_XPT_DPCR0_INTR_STATUS_REG_EN_INTR_STATUS_REG_EN_DEFAULT 0x00000000

/***************************************************************************
 *STC_EXT_CTRL - Data Transport PCR STC Extension Control Register
 ***************************************************************************/
/* XPT_DPCR0 :: STC_EXT_CTRL :: reserved0 [31:16] */
#define BCHP_XPT_DPCR0_STC_EXT_CTRL_reserved0_MASK                 0xffff0000
#define BCHP_XPT_DPCR0_STC_EXT_CTRL_reserved0_SHIFT                16

/* XPT_DPCR0 :: STC_EXT_CTRL :: PRESCALE [15:08] */
#define BCHP_XPT_DPCR0_STC_EXT_CTRL_PRESCALE_MASK                  0x0000ff00
#define BCHP_XPT_DPCR0_STC_EXT_CTRL_PRESCALE_SHIFT                 8
#define BCHP_XPT_DPCR0_STC_EXT_CTRL_PRESCALE_DEFAULT               0x00000000

/* XPT_DPCR0 :: STC_EXT_CTRL :: INC_VAL [07:00] */
#define BCHP_XPT_DPCR0_STC_EXT_CTRL_INC_VAL_MASK                   0x000000ff
#define BCHP_XPT_DPCR0_STC_EXT_CTRL_INC_VAL_SHIFT                  0
#define BCHP_XPT_DPCR0_STC_EXT_CTRL_INC_VAL_DEFAULT                0x00000001

/***************************************************************************
 *MAX_PCR_ERROR - Data Transport PCR Max PCR Error Register
 ***************************************************************************/
/* XPT_DPCR0 :: MAX_PCR_ERROR :: MAX_PCR_ERROR [31:00] */
#define BCHP_XPT_DPCR0_MAX_PCR_ERROR_MAX_PCR_ERROR_MASK            0xffffffff
#define BCHP_XPT_DPCR0_MAX_PCR_ERROR_MAX_PCR_ERROR_SHIFT           0
#define BCHP_XPT_DPCR0_MAX_PCR_ERROR_MAX_PCR_ERROR_DEFAULT         0x000000ff

/***************************************************************************
 *SEND_BASE - Data Transport PCR Send Base Register
 ***************************************************************************/
/* union - case MPEG [31:00] */
/* XPT_DPCR0 :: SEND_BASE :: MPEG :: PCR_SEND_BASE_MSBITS [31:00] */
#define BCHP_XPT_DPCR0_SEND_BASE_MPEG_PCR_SEND_BASE_MSBITS_MASK    0xffffffff
#define BCHP_XPT_DPCR0_SEND_BASE_MPEG_PCR_SEND_BASE_MSBITS_SHIFT   0
#define BCHP_XPT_DPCR0_SEND_BASE_MPEG_PCR_SEND_BASE_MSBITS_DEFAULT 0x00000000

/* union - case DIRECTV [31:00] */
/* XPT_DPCR0 :: SEND_BASE :: DIRECTV :: reserved0 [31:22] */
#define BCHP_XPT_DPCR0_SEND_BASE_DIRECTV_reserved0_MASK            0xffc00000
#define BCHP_XPT_DPCR0_SEND_BASE_DIRECTV_reserved0_SHIFT           22

/* XPT_DPCR0 :: SEND_BASE :: DIRECTV :: RTS_SEND_MSBITS [21:00] */
#define BCHP_XPT_DPCR0_SEND_BASE_DIRECTV_RTS_SEND_MSBITS_MASK      0x003fffff
#define BCHP_XPT_DPCR0_SEND_BASE_DIRECTV_RTS_SEND_MSBITS_SHIFT     0
#define BCHP_XPT_DPCR0_SEND_BASE_DIRECTV_RTS_SEND_MSBITS_DEFAULT   0x00000000

/***************************************************************************
 *SEND_EXT - Data Transport PCR Send Extension Register
 ***************************************************************************/
/* XPT_DPCR0 :: SEND_EXT :: reserved0 [31:10] */
#define BCHP_XPT_DPCR0_SEND_EXT_reserved0_MASK                     0xfffffc00
#define BCHP_XPT_DPCR0_SEND_EXT_reserved0_SHIFT                    10

/* union - case MPEG [09:00] */
/* XPT_DPCR0 :: SEND_EXT :: MPEG :: PCR_SEND_BASE_LSBIT [09:09] */
#define BCHP_XPT_DPCR0_SEND_EXT_MPEG_PCR_SEND_BASE_LSBIT_MASK      0x00000200
#define BCHP_XPT_DPCR0_SEND_EXT_MPEG_PCR_SEND_BASE_LSBIT_SHIFT     9
#define BCHP_XPT_DPCR0_SEND_EXT_MPEG_PCR_SEND_BASE_LSBIT_DEFAULT   0x00000000

/* XPT_DPCR0 :: SEND_EXT :: MPEG :: PCR_SEND_EXT [08:00] */
#define BCHP_XPT_DPCR0_SEND_EXT_MPEG_PCR_SEND_EXT_MASK             0x000001ff
#define BCHP_XPT_DPCR0_SEND_EXT_MPEG_PCR_SEND_EXT_SHIFT            0
#define BCHP_XPT_DPCR0_SEND_EXT_MPEG_PCR_SEND_EXT_DEFAULT          0x00000000

/* union - case DIRECTV [09:00] */
/* XPT_DPCR0 :: SEND_EXT :: DIRECTV :: RTS_SEND_LSBITS [09:00] */
#define BCHP_XPT_DPCR0_SEND_EXT_DIRECTV_RTS_SEND_LSBITS_MASK       0x000003ff
#define BCHP_XPT_DPCR0_SEND_EXT_DIRECTV_RTS_SEND_LSBITS_SHIFT      0
#define BCHP_XPT_DPCR0_SEND_EXT_DIRECTV_RTS_SEND_LSBITS_DEFAULT    0x00000000

/***************************************************************************
 *STC_EXT_CTRL27 - Data Transport PCR STC Extension Control Register (Test Only)
 ***************************************************************************/
/* XPT_DPCR0 :: STC_EXT_CTRL27 :: reserved0 [31:16] */
#define BCHP_XPT_DPCR0_STC_EXT_CTRL27_reserved0_MASK               0xffff0000
#define BCHP_XPT_DPCR0_STC_EXT_CTRL27_reserved0_SHIFT              16

/* XPT_DPCR0 :: STC_EXT_CTRL27 :: PCR_STC_EXT_CTRL27 [15:00] */
#define BCHP_XPT_DPCR0_STC_EXT_CTRL27_PCR_STC_EXT_CTRL27_MASK      0x0000ffff
#define BCHP_XPT_DPCR0_STC_EXT_CTRL27_PCR_STC_EXT_CTRL27_SHIFT     0

/***************************************************************************
 *STC_HI - Data Transport PCR STC MSBs Register
 ***************************************************************************/
/* XPT_DPCR0 :: STC_HI :: PCR_STC_HI [31:00] */
#define BCHP_XPT_DPCR0_STC_HI_PCR_STC_HI_MASK                      0xffffffff
#define BCHP_XPT_DPCR0_STC_HI_PCR_STC_HI_SHIFT                     0
#define BCHP_XPT_DPCR0_STC_HI_PCR_STC_HI_DEFAULT                   0x00000000

/***************************************************************************
 *STC_LO - Data Transport PCR STC LSBs Register
 ***************************************************************************/
/* XPT_DPCR0 :: STC_LO :: reserved0 [31:10] */
#define BCHP_XPT_DPCR0_STC_LO_reserved0_MASK                       0xfffffc00
#define BCHP_XPT_DPCR0_STC_LO_reserved0_SHIFT                      10

/* XPT_DPCR0 :: STC_LO :: PCR_STC_LO [09:00] */
#define BCHP_XPT_DPCR0_STC_LO_PCR_STC_LO_MASK                      0x000003ff
#define BCHP_XPT_DPCR0_STC_LO_PCR_STC_LO_SHIFT                     0
#define BCHP_XPT_DPCR0_STC_LO_PCR_STC_LO_DEFAULT                   0x00000000

/***************************************************************************
 *PWM_CTRLVALUE - Data Transport PCR PWM Control Value Register
 ***************************************************************************/
/* XPT_DPCR0 :: PWM_CTRLVALUE :: reserved0 [31:16] */
#define BCHP_XPT_DPCR0_PWM_CTRLVALUE_reserved0_MASK                0xffff0000
#define BCHP_XPT_DPCR0_PWM_CTRLVALUE_reserved0_SHIFT               16

/* XPT_DPCR0 :: PWM_CTRLVALUE :: PWM_CTRLVALUE [15:00] */
#define BCHP_XPT_DPCR0_PWM_CTRLVALUE_PWM_CTRLVALUE_MASK            0x0000ffff
#define BCHP_XPT_DPCR0_PWM_CTRLVALUE_PWM_CTRLVALUE_SHIFT           0
#define BCHP_XPT_DPCR0_PWM_CTRLVALUE_PWM_CTRLVALUE_DEFAULT         0x00000000

/***************************************************************************
 *LAST_PCR_HI - Data Transport PCR Last PCR MSBs Register
 ***************************************************************************/
/* XPT_DPCR0 :: LAST_PCR_HI :: PCR_LAST_PCR_BASE_MSBITS [31:00] */
#define BCHP_XPT_DPCR0_LAST_PCR_HI_PCR_LAST_PCR_BASE_MSBITS_MASK   0xffffffff
#define BCHP_XPT_DPCR0_LAST_PCR_HI_PCR_LAST_PCR_BASE_MSBITS_SHIFT  0
#define BCHP_XPT_DPCR0_LAST_PCR_HI_PCR_LAST_PCR_BASE_MSBITS_DEFAULT 0x00000000

/***************************************************************************
 *LAST_PCR_LO - Data Transport PCR Last PCR LSBs Register
 ***************************************************************************/
/* XPT_DPCR0 :: LAST_PCR_LO :: reserved0 [31:10] */
#define BCHP_XPT_DPCR0_LAST_PCR_LO_reserved0_MASK                  0xfffffc00
#define BCHP_XPT_DPCR0_LAST_PCR_LO_reserved0_SHIFT                 10

/* XPT_DPCR0 :: LAST_PCR_LO :: PCR_LAST_PCR_BASE_LSBITS [09:09] */
#define BCHP_XPT_DPCR0_LAST_PCR_LO_PCR_LAST_PCR_BASE_LSBITS_MASK   0x00000200
#define BCHP_XPT_DPCR0_LAST_PCR_LO_PCR_LAST_PCR_BASE_LSBITS_SHIFT  9
#define BCHP_XPT_DPCR0_LAST_PCR_LO_PCR_LAST_PCR_BASE_LSBITS_DEFAULT 0x00000000

/* XPT_DPCR0 :: LAST_PCR_LO :: PCR_LAST_PCR_EXTENSION [08:00] */
#define BCHP_XPT_DPCR0_LAST_PCR_LO_PCR_LAST_PCR_EXTENSION_MASK     0x000001ff
#define BCHP_XPT_DPCR0_LAST_PCR_LO_PCR_LAST_PCR_EXTENSION_SHIFT    0
#define BCHP_XPT_DPCR0_LAST_PCR_LO_PCR_LAST_PCR_EXTENSION_DEFAULT  0x00000000

/***************************************************************************
 *STC_BASE_LSBS - Data Transport PCR STC Base LSBs Register
 ***************************************************************************/
/* XPT_DPCR0 :: STC_BASE_LSBS :: PCR_STC_BASE_LSBS [31:00] */
#define BCHP_XPT_DPCR0_STC_BASE_LSBS_PCR_STC_BASE_LSBS_MASK        0xffffffff
#define BCHP_XPT_DPCR0_STC_BASE_LSBS_PCR_STC_BASE_LSBS_SHIFT       0
#define BCHP_XPT_DPCR0_STC_BASE_LSBS_PCR_STC_BASE_LSBS_DEFAULT     0x00000000

/***************************************************************************
 *PHASE_ERROR - Timebase Last Phase Error
 ***************************************************************************/
/* XPT_DPCR0 :: PHASE_ERROR :: reserved0 [31:22] */
#define BCHP_XPT_DPCR0_PHASE_ERROR_reserved0_MASK                  0xffc00000
#define BCHP_XPT_DPCR0_PHASE_ERROR_reserved0_SHIFT                 22

/* XPT_DPCR0 :: PHASE_ERROR :: INTEGRATOR_FREEZE [21:21] */
#define BCHP_XPT_DPCR0_PHASE_ERROR_INTEGRATOR_FREEZE_MASK          0x00200000
#define BCHP_XPT_DPCR0_PHASE_ERROR_INTEGRATOR_FREEZE_SHIFT         21

/* XPT_DPCR0 :: PHASE_ERROR :: PHASE_ERROR [20:00] */
#define BCHP_XPT_DPCR0_PHASE_ERROR_PHASE_ERROR_MASK                0x001fffff
#define BCHP_XPT_DPCR0_PHASE_ERROR_PHASE_ERROR_SHIFT               0
#define BCHP_XPT_DPCR0_PHASE_ERROR_PHASE_ERROR_DEFAULT             0x00000000

/***************************************************************************
 *LOOP_CTRL - Timebase Control
 ***************************************************************************/
/* XPT_DPCR0 :: LOOP_CTRL :: reserved0 [31:30] */
#define BCHP_XPT_DPCR0_LOOP_CTRL_reserved0_MASK                    0xc0000000
#define BCHP_XPT_DPCR0_LOOP_CTRL_reserved0_SHIFT                   30

/* XPT_DPCR0 :: LOOP_CTRL :: DITHER [29:22] */
#define BCHP_XPT_DPCR0_LOOP_CTRL_DITHER_MASK                       0x3fc00000
#define BCHP_XPT_DPCR0_LOOP_CTRL_DITHER_SHIFT                      22
#define BCHP_XPT_DPCR0_LOOP_CTRL_DITHER_DEFAULT                    0x000000ff

/* XPT_DPCR0 :: LOOP_CTRL :: FREEZE_INTEGRATOR [21:21] */
#define BCHP_XPT_DPCR0_LOOP_CTRL_FREEZE_INTEGRATOR_MASK            0x00200000
#define BCHP_XPT_DPCR0_LOOP_CTRL_FREEZE_INTEGRATOR_SHIFT           21
#define BCHP_XPT_DPCR0_LOOP_CTRL_FREEZE_INTEGRATOR_DEFAULT         0x00000000
#define BCHP_XPT_DPCR0_LOOP_CTRL_FREEZE_INTEGRATOR_FREEZE          1
#define BCHP_XPT_DPCR0_LOOP_CTRL_FREEZE_INTEGRATOR_RUN             0

/* XPT_DPCR0 :: LOOP_CTRL :: REF_POLARITY [20:19] */
#define BCHP_XPT_DPCR0_LOOP_CTRL_REF_POLARITY_MASK                 0x00180000
#define BCHP_XPT_DPCR0_LOOP_CTRL_REF_POLARITY_SHIFT                19
#define BCHP_XPT_DPCR0_LOOP_CTRL_REF_POLARITY_DEFAULT              0x00000001
#define BCHP_XPT_DPCR0_LOOP_CTRL_REF_POLARITY_NONE                 0
#define BCHP_XPT_DPCR0_LOOP_CTRL_REF_POLARITY_RISING               1
#define BCHP_XPT_DPCR0_LOOP_CTRL_REF_POLARITY_FALLING              2
#define BCHP_XPT_DPCR0_LOOP_CTRL_REF_POLARITY_BOTH                 3

/* XPT_DPCR0 :: LOOP_CTRL :: TIME_REF [18:14] */
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_MASK                     0x0007c000
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_SHIFT                    14
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_DEFAULT                  0x0000000f
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_I2S_0                    6
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_DVO_HL_0                 11
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_DVO_VL_0                 12
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_TIMEBASE_INPUT           13
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_INTERNAL                 14
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_TRANSPORT                15
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_HDMI_PTHRU_HL            16
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_HDMI_PTHRU_VL            17
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_SPDIF                    18
#define BCHP_XPT_DPCR0_LOOP_CTRL_TIME_REF_MAI_IN                   19

/* XPT_DPCR0 :: LOOP_CTRL :: TRACK_RANGE [13:11] */
#define BCHP_XPT_DPCR0_LOOP_CTRL_TRACK_RANGE_MASK                  0x00003800
#define BCHP_XPT_DPCR0_LOOP_CTRL_TRACK_RANGE_SHIFT                 11
#define BCHP_XPT_DPCR0_LOOP_CTRL_TRACK_RANGE_DEFAULT               0x00000003
#define BCHP_XPT_DPCR0_LOOP_CTRL_TRACK_RANGE_PPM_8                 0
#define BCHP_XPT_DPCR0_LOOP_CTRL_TRACK_RANGE_PPM_15                1
#define BCHP_XPT_DPCR0_LOOP_CTRL_TRACK_RANGE_PPM_30                2
#define BCHP_XPT_DPCR0_LOOP_CTRL_TRACK_RANGE_PPM_61                3
#define BCHP_XPT_DPCR0_LOOP_CTRL_TRACK_RANGE_PPM_122               4
#define BCHP_XPT_DPCR0_LOOP_CTRL_TRACK_RANGE_PPM_244               5

/* XPT_DPCR0 :: LOOP_CTRL :: INTEGRATE_MODE [10:10] */
#define BCHP_XPT_DPCR0_LOOP_CTRL_INTEGRATE_MODE_MASK               0x00000400
#define BCHP_XPT_DPCR0_LOOP_CTRL_INTEGRATE_MODE_SHIFT              10
#define BCHP_XPT_DPCR0_LOOP_CTRL_INTEGRATE_MODE_DEFAULT            0x00000000
#define BCHP_XPT_DPCR0_LOOP_CTRL_INTEGRATE_MODE_EVENT_TRIGGER      1
#define BCHP_XPT_DPCR0_LOOP_CTRL_INTEGRATE_MODE_NORMAL             0

/* XPT_DPCR0 :: LOOP_CTRL :: FILT_C [09:07] */
#define BCHP_XPT_DPCR0_LOOP_CTRL_FILT_C_MASK                       0x00000380
#define BCHP_XPT_DPCR0_LOOP_CTRL_FILT_C_SHIFT                      7
#define BCHP_XPT_DPCR0_LOOP_CTRL_FILT_C_DEFAULT                    0x00000004

/* XPT_DPCR0 :: LOOP_CTRL :: FILT_B [06:03] */
#define BCHP_XPT_DPCR0_LOOP_CTRL_FILT_B_MASK                       0x00000078
#define BCHP_XPT_DPCR0_LOOP_CTRL_FILT_B_SHIFT                      3
#define BCHP_XPT_DPCR0_LOOP_CTRL_FILT_B_DEFAULT                    0x00000008

/* XPT_DPCR0 :: LOOP_CTRL :: FILT_A [02:00] */
#define BCHP_XPT_DPCR0_LOOP_CTRL_FILT_A_MASK                       0x00000007
#define BCHP_XPT_DPCR0_LOOP_CTRL_FILT_A_SHIFT                      0
#define BCHP_XPT_DPCR0_LOOP_CTRL_FILT_A_DEFAULT                    0x00000007

/***************************************************************************
 *REF_PCR_PRESCALE - Timebase Frequency Reference Prescale Control
 ***************************************************************************/
/* XPT_DPCR0 :: REF_PCR_PRESCALE :: PRESCALE [31:00] */
#define BCHP_XPT_DPCR0_REF_PCR_PRESCALE_PRESCALE_MASK              0xffffffff
#define BCHP_XPT_DPCR0_REF_PCR_PRESCALE_PRESCALE_SHIFT             0
#define BCHP_XPT_DPCR0_REF_PCR_PRESCALE_PRESCALE_DEFAULT           0x00000000

/***************************************************************************
 *REF_PCR_INC - Timebase Frequency Reference Increment Control
 ***************************************************************************/
/* XPT_DPCR0 :: REF_PCR_INC :: PCR_INC [31:00] */
#define BCHP_XPT_DPCR0_REF_PCR_INC_PCR_INC_MASK                    0xffffffff
#define BCHP_XPT_DPCR0_REF_PCR_INC_PCR_INC_SHIFT                   0
#define BCHP_XPT_DPCR0_REF_PCR_INC_PCR_INC_DEFAULT                 0x000dbf24

/***************************************************************************
 *CENTER - Timebase Center Frequency
 ***************************************************************************/
/* XPT_DPCR0 :: CENTER :: reserved0 [31:24] */
#define BCHP_XPT_DPCR0_CENTER_reserved0_MASK                       0xff000000
#define BCHP_XPT_DPCR0_CENTER_reserved0_SHIFT                      24

/* XPT_DPCR0 :: CENTER :: CENTER [23:00] */
#define BCHP_XPT_DPCR0_CENTER_CENTER_MASK                          0x00ffffff
#define BCHP_XPT_DPCR0_CENTER_CENTER_SHIFT                         0
#define BCHP_XPT_DPCR0_CENTER_CENTER_DEFAULT                       0x00400000

/***************************************************************************
 *ACCUM_VALUE - Timebase Loop Filter Integrator
 ***************************************************************************/
/* XPT_DPCR0 :: ACCUM_VALUE :: accum_value [31:00] */
#define BCHP_XPT_DPCR0_ACCUM_VALUE_accum_value_MASK                0xffffffff
#define BCHP_XPT_DPCR0_ACCUM_VALUE_accum_value_SHIFT               0
#define BCHP_XPT_DPCR0_ACCUM_VALUE_accum_value_DEFAULT             0x00000000

/***************************************************************************
 *PCR_COUNT - Data Transport PCR Phase Error Register
 ***************************************************************************/
/* XPT_DPCR0 :: PCR_COUNT :: reserved0 [31:08] */
#define BCHP_XPT_DPCR0_PCR_COUNT_reserved0_MASK                    0xffffff00
#define BCHP_XPT_DPCR0_PCR_COUNT_reserved0_SHIFT                   8

/* XPT_DPCR0 :: PCR_COUNT :: PCR_COUNT [07:00] */
#define BCHP_XPT_DPCR0_PCR_COUNT_PCR_COUNT_MASK                    0x000000ff
#define BCHP_XPT_DPCR0_PCR_COUNT_PCR_COUNT_SHIFT                   0
#define BCHP_XPT_DPCR0_PCR_COUNT_PCR_COUNT_DEFAULT                 0x00000000

/***************************************************************************
 *SOFT_PCR_CTRL - Data Transport Soft PCR Control Register
 ***************************************************************************/
/* XPT_DPCR0 :: SOFT_PCR_CTRL :: reserved0 [31:01] */
#define BCHP_XPT_DPCR0_SOFT_PCR_CTRL_reserved0_MASK                0xfffffffe
#define BCHP_XPT_DPCR0_SOFT_PCR_CTRL_reserved0_SHIFT               1

/* XPT_DPCR0 :: SOFT_PCR_CTRL :: SOFT_PCR [00:00] */
#define BCHP_XPT_DPCR0_SOFT_PCR_CTRL_SOFT_PCR_MASK                 0x00000001
#define BCHP_XPT_DPCR0_SOFT_PCR_CTRL_SOFT_PCR_SHIFT                0
#define BCHP_XPT_DPCR0_SOFT_PCR_CTRL_SOFT_PCR_DEFAULT              0x00000000
#define BCHP_XPT_DPCR0_SOFT_PCR_CTRL_SOFT_PCR_SOFT_PCR             1
#define BCHP_XPT_DPCR0_SOFT_PCR_CTRL_SOFT_PCR_STREAM_PCR           0

/***************************************************************************
 *SOFT_PCR_BASE - Data Transport Soft PCR BASE Register
 ***************************************************************************/
/* XPT_DPCR0 :: SOFT_PCR_BASE :: SOFT_PCR_BASE_MSBITS [31:00] */
#define BCHP_XPT_DPCR0_SOFT_PCR_BASE_SOFT_PCR_BASE_MSBITS_MASK     0xffffffff
#define BCHP_XPT_DPCR0_SOFT_PCR_BASE_SOFT_PCR_BASE_MSBITS_SHIFT    0
#define BCHP_XPT_DPCR0_SOFT_PCR_BASE_SOFT_PCR_BASE_MSBITS_DEFAULT  0x00000000

/***************************************************************************
 *SOFT_PCR_EXT - Data Transport Soft PCR Extension Register
 ***************************************************************************/
/* XPT_DPCR0 :: SOFT_PCR_EXT :: reserved0 [31:10] */
#define BCHP_XPT_DPCR0_SOFT_PCR_EXT_reserved0_MASK                 0xfffffc00
#define BCHP_XPT_DPCR0_SOFT_PCR_EXT_reserved0_SHIFT                10

/* XPT_DPCR0 :: SOFT_PCR_EXT :: SOFT_PCR_BASE_LSBIT [09:09] */
#define BCHP_XPT_DPCR0_SOFT_PCR_EXT_SOFT_PCR_BASE_LSBIT_MASK       0x00000200
#define BCHP_XPT_DPCR0_SOFT_PCR_EXT_SOFT_PCR_BASE_LSBIT_SHIFT      9
#define BCHP_XPT_DPCR0_SOFT_PCR_EXT_SOFT_PCR_BASE_LSBIT_DEFAULT    0x00000000

/* XPT_DPCR0 :: SOFT_PCR_EXT :: SOFT_PCR_EXT [08:00] */
#define BCHP_XPT_DPCR0_SOFT_PCR_EXT_SOFT_PCR_EXT_MASK              0x000001ff
#define BCHP_XPT_DPCR0_SOFT_PCR_EXT_SOFT_PCR_EXT_SHIFT             0
#define BCHP_XPT_DPCR0_SOFT_PCR_EXT_SOFT_PCR_EXT_DEFAULT           0x00000000

/***************************************************************************
 *PHASE_ERROR_CLAMP - Timebase Phase Error Control
 ***************************************************************************/
/* XPT_DPCR0 :: PHASE_ERROR_CLAMP :: reserved0 [31:04] */
#define BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP_reserved0_MASK            0xfffffff0
#define BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP_reserved0_SHIFT           4

/* XPT_DPCR0 :: PHASE_ERROR_CLAMP :: PHASE_ERROR_CLAMP_RANGE [03:00] */
#define BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP_PHASE_ERROR_CLAMP_RANGE_MASK 0x0000000f
#define BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP_PHASE_ERROR_CLAMP_RANGE_SHIFT 0
#define BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP_PHASE_ERROR_CLAMP_RANGE_DEFAULT 0x0000000a

/***************************************************************************
 *TIMEBASE_INPUT_SEL - Timebase Input Select for Timebase Loop
 ***************************************************************************/
/* XPT_DPCR0 :: TIMEBASE_INPUT_SEL :: reserved0 [31:04] */
#define BCHP_XPT_DPCR0_TIMEBASE_INPUT_SEL_reserved0_MASK           0xfffffff0
#define BCHP_XPT_DPCR0_TIMEBASE_INPUT_SEL_reserved0_SHIFT          4

/* XPT_DPCR0 :: TIMEBASE_INPUT_SEL :: TIMEBASE_INPUT_SEL [03:00] */
#define BCHP_XPT_DPCR0_TIMEBASE_INPUT_SEL_TIMEBASE_INPUT_SEL_MASK  0x0000000f
#define BCHP_XPT_DPCR0_TIMEBASE_INPUT_SEL_TIMEBASE_INPUT_SEL_SHIFT 0
#define BCHP_XPT_DPCR0_TIMEBASE_INPUT_SEL_TIMEBASE_INPUT_SEL_DEFAULT 0x00000000

#endif /* #ifndef BCHP_XPT_DPCR0_H__ */

/* End of File */
