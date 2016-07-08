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
 * Date:           Generated on               Mon Feb  8 12:53:12 2016
 *                 Full Compile MD5 Checksum  7c463a9180016920b3e03273285ff33d
 *                     (minus title and desc)
 *                 MD5 Checksum               30fed0099690880293569d98807ed1d8
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     749
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_BSCD_H__
#define BCHP_BSCD_H__

/***************************************************************************
 *BSCD - Broadcom Serial Control Master D
 ***************************************************************************/
#define BCHP_BSCD_CHIP_ADDRESS                   0x0040a280 /* [RW] BSC Chip Address And Read/Write Control */
#define BCHP_BSCD_DATA_IN0                       0x0040a284 /* [RW] BSC Write Data Register 0 */
#define BCHP_BSCD_DATA_IN1                       0x0040a288 /* [RW] BSC Write Data Register 1 */
#define BCHP_BSCD_DATA_IN2                       0x0040a28c /* [RW] BSC Write Data Register 2 */
#define BCHP_BSCD_DATA_IN3                       0x0040a290 /* [RW] BSC Write Data Register 3 */
#define BCHP_BSCD_DATA_IN4                       0x0040a294 /* [RW] BSC Write Data Register 4 */
#define BCHP_BSCD_DATA_IN5                       0x0040a298 /* [RW] BSC Write Data Register 5 */
#define BCHP_BSCD_DATA_IN6                       0x0040a29c /* [RW] BSC Write Data Register 6 */
#define BCHP_BSCD_DATA_IN7                       0x0040a2a0 /* [RW] BSC Write Data Register 7 */
#define BCHP_BSCD_CNT_REG                        0x0040a2a4 /* [RW] BSC Transfer Count Register */
#define BCHP_BSCD_CTL_REG                        0x0040a2a8 /* [RW] BSC Control Register */
#define BCHP_BSCD_IIC_ENABLE                     0x0040a2ac /* [RW] BSC Read/Write Enable And Interrupt */
#define BCHP_BSCD_DATA_OUT0                      0x0040a2b0 /* [RO] BSC Read Data Register 0 */
#define BCHP_BSCD_DATA_OUT1                      0x0040a2b4 /* [RO] BSC Read Data Register 1 */
#define BCHP_BSCD_DATA_OUT2                      0x0040a2b8 /* [RO] BSC Read Data Register 2 */
#define BCHP_BSCD_DATA_OUT3                      0x0040a2bc /* [RO] BSC Read Data Register 3 */
#define BCHP_BSCD_DATA_OUT4                      0x0040a2c0 /* [RO] BSC Read Data Register 4 */
#define BCHP_BSCD_DATA_OUT5                      0x0040a2c4 /* [RO] BSC Read Data Register 5 */
#define BCHP_BSCD_DATA_OUT6                      0x0040a2c8 /* [RO] BSC Read Data Register 6 */
#define BCHP_BSCD_DATA_OUT7                      0x0040a2cc /* [RO] BSC Read Data Register 7 */
#define BCHP_BSCD_CTLHI_REG                      0x0040a2d0 /* [RW] BSC Control Register */
#define BCHP_BSCD_SCL_PARAM                      0x0040a2d4 /* [RW] BSC SCL Parameter Register */

/***************************************************************************
 *CHIP_ADDRESS - BSC Chip Address And Read/Write Control
 ***************************************************************************/
/* BSCD :: CHIP_ADDRESS :: reserved0 [31:08] */
#define BCHP_BSCD_CHIP_ADDRESS_reserved0_MASK                      0xffffff00
#define BCHP_BSCD_CHIP_ADDRESS_reserved0_SHIFT                     8

/* BSCD :: CHIP_ADDRESS :: CHIP_ADDRESS [07:01] */
#define BCHP_BSCD_CHIP_ADDRESS_CHIP_ADDRESS_MASK                   0x000000fe
#define BCHP_BSCD_CHIP_ADDRESS_CHIP_ADDRESS_SHIFT                  1
#define BCHP_BSCD_CHIP_ADDRESS_CHIP_ADDRESS_DEFAULT                0x00000000

/* BSCD :: CHIP_ADDRESS :: SPARE [00:00] */
#define BCHP_BSCD_CHIP_ADDRESS_SPARE_MASK                          0x00000001
#define BCHP_BSCD_CHIP_ADDRESS_SPARE_SHIFT                         0
#define BCHP_BSCD_CHIP_ADDRESS_SPARE_DEFAULT                       0x00000000

/***************************************************************************
 *DATA_IN0 - BSC Write Data Register 0
 ***************************************************************************/
/* BSCD :: DATA_IN0 :: DATA_IN0 [31:00] */
#define BCHP_BSCD_DATA_IN0_DATA_IN0_MASK                           0xffffffff
#define BCHP_BSCD_DATA_IN0_DATA_IN0_SHIFT                          0

/***************************************************************************
 *DATA_IN1 - BSC Write Data Register 1
 ***************************************************************************/
/* BSCD :: DATA_IN1 :: DATA_IN1 [31:00] */
#define BCHP_BSCD_DATA_IN1_DATA_IN1_MASK                           0xffffffff
#define BCHP_BSCD_DATA_IN1_DATA_IN1_SHIFT                          0

/***************************************************************************
 *DATA_IN2 - BSC Write Data Register 2
 ***************************************************************************/
/* BSCD :: DATA_IN2 :: DATA_IN2 [31:00] */
#define BCHP_BSCD_DATA_IN2_DATA_IN2_MASK                           0xffffffff
#define BCHP_BSCD_DATA_IN2_DATA_IN2_SHIFT                          0

/***************************************************************************
 *DATA_IN3 - BSC Write Data Register 3
 ***************************************************************************/
/* BSCD :: DATA_IN3 :: DATA_IN3 [31:00] */
#define BCHP_BSCD_DATA_IN3_DATA_IN3_MASK                           0xffffffff
#define BCHP_BSCD_DATA_IN3_DATA_IN3_SHIFT                          0

/***************************************************************************
 *DATA_IN4 - BSC Write Data Register 4
 ***************************************************************************/
/* BSCD :: DATA_IN4 :: DATA_IN4 [31:00] */
#define BCHP_BSCD_DATA_IN4_DATA_IN4_MASK                           0xffffffff
#define BCHP_BSCD_DATA_IN4_DATA_IN4_SHIFT                          0

/***************************************************************************
 *DATA_IN5 - BSC Write Data Register 5
 ***************************************************************************/
/* BSCD :: DATA_IN5 :: DATA_IN5 [31:00] */
#define BCHP_BSCD_DATA_IN5_DATA_IN5_MASK                           0xffffffff
#define BCHP_BSCD_DATA_IN5_DATA_IN5_SHIFT                          0

/***************************************************************************
 *DATA_IN6 - BSC Write Data Register 6
 ***************************************************************************/
/* BSCD :: DATA_IN6 :: DATA_IN6 [31:00] */
#define BCHP_BSCD_DATA_IN6_DATA_IN6_MASK                           0xffffffff
#define BCHP_BSCD_DATA_IN6_DATA_IN6_SHIFT                          0

/***************************************************************************
 *DATA_IN7 - BSC Write Data Register 7
 ***************************************************************************/
/* BSCD :: DATA_IN7 :: DATA_IN7 [31:00] */
#define BCHP_BSCD_DATA_IN7_DATA_IN7_MASK                           0xffffffff
#define BCHP_BSCD_DATA_IN7_DATA_IN7_SHIFT                          0

/***************************************************************************
 *CNT_REG - BSC Transfer Count Register
 ***************************************************************************/
/* BSCD :: CNT_REG :: reserved0 [31:12] */
#define BCHP_BSCD_CNT_REG_reserved0_MASK                           0xfffff000
#define BCHP_BSCD_CNT_REG_reserved0_SHIFT                          12

/* BSCD :: CNT_REG :: CNT_REG2 [11:06] */
#define BCHP_BSCD_CNT_REG_CNT_REG2_MASK                            0x00000fc0
#define BCHP_BSCD_CNT_REG_CNT_REG2_SHIFT                           6
#define BCHP_BSCD_CNT_REG_CNT_REG2_DEFAULT                         0x00000000

/* BSCD :: CNT_REG :: CNT_REG1 [05:00] */
#define BCHP_BSCD_CNT_REG_CNT_REG1_MASK                            0x0000003f
#define BCHP_BSCD_CNT_REG_CNT_REG1_SHIFT                           0
#define BCHP_BSCD_CNT_REG_CNT_REG1_DEFAULT                         0x00000000

/***************************************************************************
 *CTL_REG - BSC Control Register
 ***************************************************************************/
/* BSCD :: CTL_REG :: reserved0 [31:11] */
#define BCHP_BSCD_CTL_REG_reserved0_MASK                           0xfffff800
#define BCHP_BSCD_CTL_REG_reserved0_SHIFT                          11

/* BSCD :: CTL_REG :: SDA_DELAY_SEL [10:08] */
#define BCHP_BSCD_CTL_REG_SDA_DELAY_SEL_MASK                       0x00000700
#define BCHP_BSCD_CTL_REG_SDA_DELAY_SEL_SHIFT                      8
#define BCHP_BSCD_CTL_REG_SDA_DELAY_SEL_DEFAULT                    0x00000000

/* BSCD :: CTL_REG :: DIV_CLK [07:07] */
#define BCHP_BSCD_CTL_REG_DIV_CLK_MASK                             0x00000080
#define BCHP_BSCD_CTL_REG_DIV_CLK_SHIFT                            7
#define BCHP_BSCD_CTL_REG_DIV_CLK_DEFAULT                          0x00000000

/* BSCD :: CTL_REG :: INT_EN [06:06] */
#define BCHP_BSCD_CTL_REG_INT_EN_MASK                              0x00000040
#define BCHP_BSCD_CTL_REG_INT_EN_SHIFT                             6
#define BCHP_BSCD_CTL_REG_INT_EN_DEFAULT                           0x00000000

/* BSCD :: CTL_REG :: SCL_SEL [05:04] */
#define BCHP_BSCD_CTL_REG_SCL_SEL_MASK                             0x00000030
#define BCHP_BSCD_CTL_REG_SCL_SEL_SHIFT                            4
#define BCHP_BSCD_CTL_REG_SCL_SEL_DEFAULT                          0x00000000

/* BSCD :: CTL_REG :: DELAY_DIS [03:03] */
#define BCHP_BSCD_CTL_REG_DELAY_DIS_MASK                           0x00000008
#define BCHP_BSCD_CTL_REG_DELAY_DIS_SHIFT                          3
#define BCHP_BSCD_CTL_REG_DELAY_DIS_DEFAULT                        0x00000000

/* BSCD :: CTL_REG :: DEGLITCH_DIS [02:02] */
#define BCHP_BSCD_CTL_REG_DEGLITCH_DIS_MASK                        0x00000004
#define BCHP_BSCD_CTL_REG_DEGLITCH_DIS_SHIFT                       2
#define BCHP_BSCD_CTL_REG_DEGLITCH_DIS_DEFAULT                     0x00000000

/* BSCD :: CTL_REG :: DTF [01:00] */
#define BCHP_BSCD_CTL_REG_DTF_MASK                                 0x00000003
#define BCHP_BSCD_CTL_REG_DTF_SHIFT                                0
#define BCHP_BSCD_CTL_REG_DTF_DEFAULT                              0x00000000

/***************************************************************************
 *IIC_ENABLE - BSC Read/Write Enable And Interrupt
 ***************************************************************************/
/* BSCD :: IIC_ENABLE :: reserved0 [31:07] */
#define BCHP_BSCD_IIC_ENABLE_reserved0_MASK                        0xffffff80
#define BCHP_BSCD_IIC_ENABLE_reserved0_SHIFT                       7

/* BSCD :: IIC_ENABLE :: RESTART [06:06] */
#define BCHP_BSCD_IIC_ENABLE_RESTART_MASK                          0x00000040
#define BCHP_BSCD_IIC_ENABLE_RESTART_SHIFT                         6
#define BCHP_BSCD_IIC_ENABLE_RESTART_DEFAULT                       0x00000000

/* BSCD :: IIC_ENABLE :: NO_START [05:05] */
#define BCHP_BSCD_IIC_ENABLE_NO_START_MASK                         0x00000020
#define BCHP_BSCD_IIC_ENABLE_NO_START_SHIFT                        5
#define BCHP_BSCD_IIC_ENABLE_NO_START_DEFAULT                      0x00000000

/* BSCD :: IIC_ENABLE :: NO_STOP [04:04] */
#define BCHP_BSCD_IIC_ENABLE_NO_STOP_MASK                          0x00000010
#define BCHP_BSCD_IIC_ENABLE_NO_STOP_SHIFT                         4
#define BCHP_BSCD_IIC_ENABLE_NO_STOP_DEFAULT                       0x00000000

/* BSCD :: IIC_ENABLE :: reserved1 [03:03] */
#define BCHP_BSCD_IIC_ENABLE_reserved1_MASK                        0x00000008
#define BCHP_BSCD_IIC_ENABLE_reserved1_SHIFT                       3

/* BSCD :: IIC_ENABLE :: NO_ACK [02:02] */
#define BCHP_BSCD_IIC_ENABLE_NO_ACK_MASK                           0x00000004
#define BCHP_BSCD_IIC_ENABLE_NO_ACK_SHIFT                          2
#define BCHP_BSCD_IIC_ENABLE_NO_ACK_DEFAULT                        0x00000000

/* BSCD :: IIC_ENABLE :: INTRP [01:01] */
#define BCHP_BSCD_IIC_ENABLE_INTRP_MASK                            0x00000002
#define BCHP_BSCD_IIC_ENABLE_INTRP_SHIFT                           1
#define BCHP_BSCD_IIC_ENABLE_INTRP_DEFAULT                         0x00000000

/* BSCD :: IIC_ENABLE :: ENABLE [00:00] */
#define BCHP_BSCD_IIC_ENABLE_ENABLE_MASK                           0x00000001
#define BCHP_BSCD_IIC_ENABLE_ENABLE_SHIFT                          0
#define BCHP_BSCD_IIC_ENABLE_ENABLE_DEFAULT                        0x00000000

/***************************************************************************
 *DATA_OUT0 - BSC Read Data Register 0
 ***************************************************************************/
/* BSCD :: DATA_OUT0 :: DATA_OUT0 [31:00] */
#define BCHP_BSCD_DATA_OUT0_DATA_OUT0_MASK                         0xffffffff
#define BCHP_BSCD_DATA_OUT0_DATA_OUT0_SHIFT                        0

/***************************************************************************
 *DATA_OUT1 - BSC Read Data Register 1
 ***************************************************************************/
/* BSCD :: DATA_OUT1 :: DATA_OUT1 [31:00] */
#define BCHP_BSCD_DATA_OUT1_DATA_OUT1_MASK                         0xffffffff
#define BCHP_BSCD_DATA_OUT1_DATA_OUT1_SHIFT                        0

/***************************************************************************
 *DATA_OUT2 - BSC Read Data Register 2
 ***************************************************************************/
/* BSCD :: DATA_OUT2 :: DATA_OUT2 [31:00] */
#define BCHP_BSCD_DATA_OUT2_DATA_OUT2_MASK                         0xffffffff
#define BCHP_BSCD_DATA_OUT2_DATA_OUT2_SHIFT                        0

/***************************************************************************
 *DATA_OUT3 - BSC Read Data Register 3
 ***************************************************************************/
/* BSCD :: DATA_OUT3 :: DATA_OUT3 [31:00] */
#define BCHP_BSCD_DATA_OUT3_DATA_OUT3_MASK                         0xffffffff
#define BCHP_BSCD_DATA_OUT3_DATA_OUT3_SHIFT                        0

/***************************************************************************
 *DATA_OUT4 - BSC Read Data Register 4
 ***************************************************************************/
/* BSCD :: DATA_OUT4 :: DATA_OUT4 [31:00] */
#define BCHP_BSCD_DATA_OUT4_DATA_OUT4_MASK                         0xffffffff
#define BCHP_BSCD_DATA_OUT4_DATA_OUT4_SHIFT                        0

/***************************************************************************
 *DATA_OUT5 - BSC Read Data Register 5
 ***************************************************************************/
/* BSCD :: DATA_OUT5 :: DATA_OUT5 [31:00] */
#define BCHP_BSCD_DATA_OUT5_DATA_OUT5_MASK                         0xffffffff
#define BCHP_BSCD_DATA_OUT5_DATA_OUT5_SHIFT                        0

/***************************************************************************
 *DATA_OUT6 - BSC Read Data Register 6
 ***************************************************************************/
/* BSCD :: DATA_OUT6 :: DATA_OUT6 [31:00] */
#define BCHP_BSCD_DATA_OUT6_DATA_OUT6_MASK                         0xffffffff
#define BCHP_BSCD_DATA_OUT6_DATA_OUT6_SHIFT                        0

/***************************************************************************
 *DATA_OUT7 - BSC Read Data Register 7
 ***************************************************************************/
/* BSCD :: DATA_OUT7 :: DATA_OUT7 [31:00] */
#define BCHP_BSCD_DATA_OUT7_DATA_OUT7_MASK                         0xffffffff
#define BCHP_BSCD_DATA_OUT7_DATA_OUT7_SHIFT                        0

/***************************************************************************
 *CTLHI_REG - BSC Control Register
 ***************************************************************************/
/* BSCD :: CTLHI_REG :: reserved0 [31:08] */
#define BCHP_BSCD_CTLHI_REG_reserved0_MASK                         0xffffff00
#define BCHP_BSCD_CTLHI_REG_reserved0_SHIFT                        8

/* BSCD :: CTLHI_REG :: INPUT_SWITCHING_LEVEL [07:07] */
#define BCHP_BSCD_CTLHI_REG_INPUT_SWITCHING_LEVEL_MASK             0x00000080
#define BCHP_BSCD_CTLHI_REG_INPUT_SWITCHING_LEVEL_SHIFT            7
#define BCHP_BSCD_CTLHI_REG_INPUT_SWITCHING_LEVEL_DEFAULT          0x00000000

/* BSCD :: CTLHI_REG :: DATA_REG_SIZE [06:06] */
#define BCHP_BSCD_CTLHI_REG_DATA_REG_SIZE_MASK                     0x00000040
#define BCHP_BSCD_CTLHI_REG_DATA_REG_SIZE_SHIFT                    6
#define BCHP_BSCD_CTLHI_REG_DATA_REG_SIZE_DEFAULT                  0x00000000

/* BSCD :: CTLHI_REG :: reserved1 [05:02] */
#define BCHP_BSCD_CTLHI_REG_reserved1_MASK                         0x0000003c
#define BCHP_BSCD_CTLHI_REG_reserved1_SHIFT                        2

/* BSCD :: CTLHI_REG :: IGNORE_ACK [01:01] */
#define BCHP_BSCD_CTLHI_REG_IGNORE_ACK_MASK                        0x00000002
#define BCHP_BSCD_CTLHI_REG_IGNORE_ACK_SHIFT                       1
#define BCHP_BSCD_CTLHI_REG_IGNORE_ACK_DEFAULT                     0x00000000

/* BSCD :: CTLHI_REG :: WAIT_DIS [00:00] */
#define BCHP_BSCD_CTLHI_REG_WAIT_DIS_MASK                          0x00000001
#define BCHP_BSCD_CTLHI_REG_WAIT_DIS_SHIFT                         0
#define BCHP_BSCD_CTLHI_REG_WAIT_DIS_DEFAULT                       0x00000000

/***************************************************************************
 *SCL_PARAM - BSC SCL Parameter Register
 ***************************************************************************/
/* BSCD :: SCL_PARAM :: reserved_for_eco0 [31:00] */
#define BCHP_BSCD_SCL_PARAM_reserved_for_eco0_MASK                 0xffffffff
#define BCHP_BSCD_SCL_PARAM_reserved_for_eco0_SHIFT                0
#define BCHP_BSCD_SCL_PARAM_reserved_for_eco0_DEFAULT              0x00000000

#endif /* #ifndef BCHP_BSCD_H__ */

/* End of File */
