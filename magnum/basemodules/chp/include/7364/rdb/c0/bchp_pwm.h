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
 * Date:           Generated on               Mon Feb  8 12:53:14 2016
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

#ifndef BCHP_PWM_H__
#define BCHP_PWM_H__

/***************************************************************************
 *PWM - PWM
 ***************************************************************************/
#define BCHP_PWM_CTRL                            0x00408000 /* [RW] PWM Control */
#define BCHP_PWM_CTRL2                           0x00408004 /* [RW] PWM Control 2 */
#define BCHP_PWM_PWM1_CWORD_MSB                  0x00408008 /* [RW] PWM 1 Frequency Control Word MSB */
#define BCHP_PWM_PWM1_CWORD_LSB                  0x0040800c /* [RW] PWM 1 Frequency Control Word LSB */
#define BCHP_PWM_PWM2_CWORD_MSB                  0x00408010 /* [RW] PWM 2 Frequency Control Word MSB */
#define BCHP_PWM_PWM2_CWORD_LSB                  0x00408014 /* [RW] PWM 2 Frequency Control Word LSB */
#define BCHP_PWM_PWM1_ON                         0x00408018 /* [RW] PWM 1 CFPWM On Interval */
#define BCHP_PWM_PWM1_PERIOD                     0x0040801c /* [RW] PWM 1 CFPWM Period */
#define BCHP_PWM_PWM2_ON                         0x00408020 /* [RW] PWM 2 CFPWM On Interval */
#define BCHP_PWM_PWM2_PERIOD                     0x00408024 /* [RW] PWM 2 CFPWM Period */

/***************************************************************************
 *CTRL - PWM Control
 ***************************************************************************/
/* PWM :: CTRL :: reserved0 [31:08] */
#define BCHP_PWM_CTRL_reserved0_MASK                               0xffffff00
#define BCHP_PWM_CTRL_reserved0_SHIFT                              8

/* PWM :: CTRL :: pwm2_opendrainb [07:07] */
#define BCHP_PWM_CTRL_pwm2_opendrainb_MASK                         0x00000080
#define BCHP_PWM_CTRL_pwm2_opendrainb_SHIFT                        7
#define BCHP_PWM_CTRL_pwm2_opendrainb_DEFAULT                      0x00000000

/* PWM :: CTRL :: pwm2_force_high [06:06] */
#define BCHP_PWM_CTRL_pwm2_force_high_MASK                         0x00000040
#define BCHP_PWM_CTRL_pwm2_force_high_SHIFT                        6
#define BCHP_PWM_CTRL_pwm2_force_high_DEFAULT                      0x00000000

/* PWM :: CTRL :: pwm2_oeb [05:05] */
#define BCHP_PWM_CTRL_pwm2_oeb_MASK                                0x00000020
#define BCHP_PWM_CTRL_pwm2_oeb_SHIFT                               5
#define BCHP_PWM_CTRL_pwm2_oeb_DEFAULT                             0x00000001

/* PWM :: CTRL :: pwm2_start [04:04] */
#define BCHP_PWM_CTRL_pwm2_start_MASK                              0x00000010
#define BCHP_PWM_CTRL_pwm2_start_SHIFT                             4
#define BCHP_PWM_CTRL_pwm2_start_DEFAULT                           0x00000000

/* PWM :: CTRL :: pwm1_opendrainb [03:03] */
#define BCHP_PWM_CTRL_pwm1_opendrainb_MASK                         0x00000008
#define BCHP_PWM_CTRL_pwm1_opendrainb_SHIFT                        3
#define BCHP_PWM_CTRL_pwm1_opendrainb_DEFAULT                      0x00000000

/* PWM :: CTRL :: pwm1_force_high [02:02] */
#define BCHP_PWM_CTRL_pwm1_force_high_MASK                         0x00000004
#define BCHP_PWM_CTRL_pwm1_force_high_SHIFT                        2
#define BCHP_PWM_CTRL_pwm1_force_high_DEFAULT                      0x00000000

/* PWM :: CTRL :: pwm1_oeb [01:01] */
#define BCHP_PWM_CTRL_pwm1_oeb_MASK                                0x00000002
#define BCHP_PWM_CTRL_pwm1_oeb_SHIFT                               1
#define BCHP_PWM_CTRL_pwm1_oeb_DEFAULT                             0x00000001

/* PWM :: CTRL :: pwm1_start [00:00] */
#define BCHP_PWM_CTRL_pwm1_start_MASK                              0x00000001
#define BCHP_PWM_CTRL_pwm1_start_SHIFT                             0
#define BCHP_PWM_CTRL_pwm1_start_DEFAULT                           0x00000000

/***************************************************************************
 *CTRL2 - PWM Control 2
 ***************************************************************************/
/* PWM :: CTRL2 :: reserved0 [31:05] */
#define BCHP_PWM_CTRL2_reserved0_MASK                              0xffffffe0
#define BCHP_PWM_CTRL2_reserved0_SHIFT                             5

/* PWM :: CTRL2 :: pwm2_out_select [04:04] */
#define BCHP_PWM_CTRL2_pwm2_out_select_MASK                        0x00000010
#define BCHP_PWM_CTRL2_pwm2_out_select_SHIFT                       4
#define BCHP_PWM_CTRL2_pwm2_out_select_DEFAULT                     0x00000000

/* PWM :: CTRL2 :: reserved1 [03:01] */
#define BCHP_PWM_CTRL2_reserved1_MASK                              0x0000000e
#define BCHP_PWM_CTRL2_reserved1_SHIFT                             1

/* PWM :: CTRL2 :: pwm1_out_select [00:00] */
#define BCHP_PWM_CTRL2_pwm1_out_select_MASK                        0x00000001
#define BCHP_PWM_CTRL2_pwm1_out_select_SHIFT                       0
#define BCHP_PWM_CTRL2_pwm1_out_select_DEFAULT                     0x00000000

/***************************************************************************
 *PWM1_CWORD_MSB - PWM 1 Frequency Control Word MSB
 ***************************************************************************/
/* PWM :: PWM1_CWORD_MSB :: reserved0 [31:08] */
#define BCHP_PWM_PWM1_CWORD_MSB_reserved0_MASK                     0xffffff00
#define BCHP_PWM_PWM1_CWORD_MSB_reserved0_SHIFT                    8

/* PWM :: PWM1_CWORD_MSB :: pwm1_cword_msb [07:00] */
#define BCHP_PWM_PWM1_CWORD_MSB_pwm1_cword_msb_MASK                0x000000ff
#define BCHP_PWM_PWM1_CWORD_MSB_pwm1_cword_msb_SHIFT               0
#define BCHP_PWM_PWM1_CWORD_MSB_pwm1_cword_msb_DEFAULT             0x00000000

/***************************************************************************
 *PWM1_CWORD_LSB - PWM 1 Frequency Control Word LSB
 ***************************************************************************/
/* PWM :: PWM1_CWORD_LSB :: reserved0 [31:08] */
#define BCHP_PWM_PWM1_CWORD_LSB_reserved0_MASK                     0xffffff00
#define BCHP_PWM_PWM1_CWORD_LSB_reserved0_SHIFT                    8

/* PWM :: PWM1_CWORD_LSB :: pwm1_cword_lsb [07:00] */
#define BCHP_PWM_PWM1_CWORD_LSB_pwm1_cword_lsb_MASK                0x000000ff
#define BCHP_PWM_PWM1_CWORD_LSB_pwm1_cword_lsb_SHIFT               0
#define BCHP_PWM_PWM1_CWORD_LSB_pwm1_cword_lsb_DEFAULT             0x00000000

/***************************************************************************
 *PWM2_CWORD_MSB - PWM 2 Frequency Control Word MSB
 ***************************************************************************/
/* PWM :: PWM2_CWORD_MSB :: reserved0 [31:08] */
#define BCHP_PWM_PWM2_CWORD_MSB_reserved0_MASK                     0xffffff00
#define BCHP_PWM_PWM2_CWORD_MSB_reserved0_SHIFT                    8

/* PWM :: PWM2_CWORD_MSB :: pwm2_cword_msb [07:00] */
#define BCHP_PWM_PWM2_CWORD_MSB_pwm2_cword_msb_MASK                0x000000ff
#define BCHP_PWM_PWM2_CWORD_MSB_pwm2_cword_msb_SHIFT               0
#define BCHP_PWM_PWM2_CWORD_MSB_pwm2_cword_msb_DEFAULT             0x00000000

/***************************************************************************
 *PWM2_CWORD_LSB - PWM 2 Frequency Control Word LSB
 ***************************************************************************/
/* PWM :: PWM2_CWORD_LSB :: reserved0 [31:08] */
#define BCHP_PWM_PWM2_CWORD_LSB_reserved0_MASK                     0xffffff00
#define BCHP_PWM_PWM2_CWORD_LSB_reserved0_SHIFT                    8

/* PWM :: PWM2_CWORD_LSB :: pwm2_cword_lsb [07:00] */
#define BCHP_PWM_PWM2_CWORD_LSB_pwm2_cword_lsb_MASK                0x000000ff
#define BCHP_PWM_PWM2_CWORD_LSB_pwm2_cword_lsb_SHIFT               0
#define BCHP_PWM_PWM2_CWORD_LSB_pwm2_cword_lsb_DEFAULT             0x00000000

/***************************************************************************
 *PWM1_ON - PWM 1 CFPWM On Interval
 ***************************************************************************/
/* PWM :: PWM1_ON :: reserved0 [31:08] */
#define BCHP_PWM_PWM1_ON_reserved0_MASK                            0xffffff00
#define BCHP_PWM_PWM1_ON_reserved0_SHIFT                           8

/* PWM :: PWM1_ON :: pwm1_on [07:00] */
#define BCHP_PWM_PWM1_ON_pwm1_on_MASK                              0x000000ff
#define BCHP_PWM_PWM1_ON_pwm1_on_SHIFT                             0
#define BCHP_PWM_PWM1_ON_pwm1_on_DEFAULT                           0x00000000

/***************************************************************************
 *PWM1_PERIOD - PWM 1 CFPWM Period
 ***************************************************************************/
/* PWM :: PWM1_PERIOD :: reserved0 [31:08] */
#define BCHP_PWM_PWM1_PERIOD_reserved0_MASK                        0xffffff00
#define BCHP_PWM_PWM1_PERIOD_reserved0_SHIFT                       8

/* PWM :: PWM1_PERIOD :: pwm1_period [07:00] */
#define BCHP_PWM_PWM1_PERIOD_pwm1_period_MASK                      0x000000ff
#define BCHP_PWM_PWM1_PERIOD_pwm1_period_SHIFT                     0
#define BCHP_PWM_PWM1_PERIOD_pwm1_period_DEFAULT                   0x00000000

/***************************************************************************
 *PWM2_ON - PWM 2 CFPWM On Interval
 ***************************************************************************/
/* PWM :: PWM2_ON :: reserved0 [31:08] */
#define BCHP_PWM_PWM2_ON_reserved0_MASK                            0xffffff00
#define BCHP_PWM_PWM2_ON_reserved0_SHIFT                           8

/* PWM :: PWM2_ON :: pwm2_on [07:00] */
#define BCHP_PWM_PWM2_ON_pwm2_on_MASK                              0x000000ff
#define BCHP_PWM_PWM2_ON_pwm2_on_SHIFT                             0
#define BCHP_PWM_PWM2_ON_pwm2_on_DEFAULT                           0x00000000

/***************************************************************************
 *PWM2_PERIOD - PWM 2 CFPWM Period
 ***************************************************************************/
/* PWM :: PWM2_PERIOD :: reserved0 [31:08] */
#define BCHP_PWM_PWM2_PERIOD_reserved0_MASK                        0xffffff00
#define BCHP_PWM_PWM2_PERIOD_reserved0_SHIFT                       8

/* PWM :: PWM2_PERIOD :: pwm2_period [07:00] */
#define BCHP_PWM_PWM2_PERIOD_pwm2_period_MASK                      0x000000ff
#define BCHP_PWM_PWM2_PERIOD_pwm2_period_SHIFT                     0
#define BCHP_PWM_PWM2_PERIOD_pwm2_period_DEFAULT                   0x00000000

#endif /* #ifndef BCHP_PWM_H__ */

/* End of File */
