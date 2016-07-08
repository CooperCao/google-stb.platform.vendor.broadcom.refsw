/********************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Fri Apr  8 10:05:33 2016
 *                 Full Compile MD5 Checksum  fedc0c202f37d89a0f031183f2971dd5
 *                     (minus title and desc)
 *                 MD5 Checksum               46ee8b9f88da54e2e54f3751e0c3b177
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     899
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_LEAP_CTRL_TIMERS_H__
#define BCHP_LEAP_CTRL_TIMERS_H__

/***************************************************************************
 *LEAP_CTRL_TIMERS - LEAP Timers and Control Registers
 ***************************************************************************/
#define BCHP_LEAP_CTRL_TIMERS_TIMER0             0x00101200 /* [RW] Timer0 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_COUNT       0x00101204 /* [RO] Timer0 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER1             0x00101208 /* [RW] Timer1 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_COUNT       0x0010120c /* [RO] Timer1 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER2             0x00101210 /* [RW] Timer2 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_COUNT       0x00101214 /* [RO] Timer2 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER3             0x00101218 /* [RW] Timer3 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_COUNT       0x0010121c /* [RO] Timer3 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER4             0x00101220 /* [RW] Timer4 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_COUNT       0x00101224 /* [RO] Timer4 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER5             0x00101228 /* [RW] Timer5 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_COUNT       0x0010122c /* [RO] Timer5 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER6             0x00101230 /* [RW] Timer6 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_COUNT       0x00101234 /* [RO] Timer6 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER7             0x00101238 /* [RW] Timer7 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_COUNT       0x0010123c /* [RO] Timer7 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER8             0x00101240 /* [RW] Timer8 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_COUNT       0x00101244 /* [RO] Timer8 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER9             0x00101248 /* [RW] Timer9 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_COUNT       0x0010124c /* [RO] Timer9 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER10            0x00101250 /* [RW] Timer10 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_COUNT      0x00101254 /* [RO] Timer10 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER11            0x00101258 /* [RW] Timer11 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_COUNT      0x0010125c /* [RO] Timer11 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER12            0x00101260 /* [RW] Timer12 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_COUNT      0x00101264 /* [RO] Timer12 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER13            0x00101268 /* [RW] Timer13 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_COUNT      0x0010126c /* [RO] Timer13 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER14            0x00101270 /* [RW] Timer14 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_COUNT      0x00101274 /* [RO] Timer14 Current Count */
#define BCHP_LEAP_CTRL_TIMERS_TIMER15            0x00101278 /* [RW] Timer15 Control Register */
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_COUNT      0x0010127c /* [RO] Timer15 Current Count */

/***************************************************************************
 *TIMER0 - Timer0 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER0 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_ENABLE_MASK                   0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_ENABLE_SHIFT                  31
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_ENABLE_DEFAULT                0x00000000

/* LEAP_CTRL_TIMERS :: TIMER0 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_PERIODIC_MASK                 0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_PERIODIC_SHIFT                30
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_PERIODIC_DEFAULT              0x00000000

/* LEAP_CTRL_TIMERS :: TIMER0 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_COMPARE_MASK                  0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_COMPARE_SHIFT                 0
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_COMPARE_DEFAULT               0x00000000

/***************************************************************************
 *TIMER0_COUNT - Timer0 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER0_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_COUNT_reserved0_MASK          0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_COUNT_reserved0_SHIFT         30

/* LEAP_CTRL_TIMERS :: TIMER0_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_COUNT_COUNT_MASK              0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_COUNT_COUNT_SHIFT             0
#define BCHP_LEAP_CTRL_TIMERS_TIMER0_COUNT_COUNT_DEFAULT           0x00000000

/***************************************************************************
 *TIMER1 - Timer1 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER1 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_ENABLE_MASK                   0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_ENABLE_SHIFT                  31
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_ENABLE_DEFAULT                0x00000000

/* LEAP_CTRL_TIMERS :: TIMER1 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_PERIODIC_MASK                 0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_PERIODIC_SHIFT                30
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_PERIODIC_DEFAULT              0x00000000

/* LEAP_CTRL_TIMERS :: TIMER1 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_COMPARE_MASK                  0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_COMPARE_SHIFT                 0
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_COMPARE_DEFAULT               0x00000000

/***************************************************************************
 *TIMER1_COUNT - Timer1 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER1_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_COUNT_reserved0_MASK          0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_COUNT_reserved0_SHIFT         30

/* LEAP_CTRL_TIMERS :: TIMER1_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_COUNT_COUNT_MASK              0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_COUNT_COUNT_SHIFT             0
#define BCHP_LEAP_CTRL_TIMERS_TIMER1_COUNT_COUNT_DEFAULT           0x00000000

/***************************************************************************
 *TIMER2 - Timer2 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER2 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_ENABLE_MASK                   0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_ENABLE_SHIFT                  31
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_ENABLE_DEFAULT                0x00000000

/* LEAP_CTRL_TIMERS :: TIMER2 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_PERIODIC_MASK                 0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_PERIODIC_SHIFT                30
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_PERIODIC_DEFAULT              0x00000000

/* LEAP_CTRL_TIMERS :: TIMER2 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_COMPARE_MASK                  0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_COMPARE_SHIFT                 0
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_COMPARE_DEFAULT               0x00000000

/***************************************************************************
 *TIMER2_COUNT - Timer2 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER2_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_COUNT_reserved0_MASK          0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_COUNT_reserved0_SHIFT         30

/* LEAP_CTRL_TIMERS :: TIMER2_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_COUNT_COUNT_MASK              0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_COUNT_COUNT_SHIFT             0
#define BCHP_LEAP_CTRL_TIMERS_TIMER2_COUNT_COUNT_DEFAULT           0x00000000

/***************************************************************************
 *TIMER3 - Timer3 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER3 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_ENABLE_MASK                   0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_ENABLE_SHIFT                  31
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_ENABLE_DEFAULT                0x00000000

/* LEAP_CTRL_TIMERS :: TIMER3 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_PERIODIC_MASK                 0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_PERIODIC_SHIFT                30
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_PERIODIC_DEFAULT              0x00000000

/* LEAP_CTRL_TIMERS :: TIMER3 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_COMPARE_MASK                  0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_COMPARE_SHIFT                 0
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_COMPARE_DEFAULT               0x00000000

/***************************************************************************
 *TIMER3_COUNT - Timer3 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER3_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_COUNT_reserved0_MASK          0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_COUNT_reserved0_SHIFT         30

/* LEAP_CTRL_TIMERS :: TIMER3_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_COUNT_COUNT_MASK              0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_COUNT_COUNT_SHIFT             0
#define BCHP_LEAP_CTRL_TIMERS_TIMER3_COUNT_COUNT_DEFAULT           0x00000000

/***************************************************************************
 *TIMER4 - Timer4 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER4 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_ENABLE_MASK                   0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_ENABLE_SHIFT                  31
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_ENABLE_DEFAULT                0x00000000

/* LEAP_CTRL_TIMERS :: TIMER4 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_PERIODIC_MASK                 0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_PERIODIC_SHIFT                30
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_PERIODIC_DEFAULT              0x00000000

/* LEAP_CTRL_TIMERS :: TIMER4 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_COMPARE_MASK                  0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_COMPARE_SHIFT                 0
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_COMPARE_DEFAULT               0x00000000

/***************************************************************************
 *TIMER4_COUNT - Timer4 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER4_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_COUNT_reserved0_MASK          0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_COUNT_reserved0_SHIFT         30

/* LEAP_CTRL_TIMERS :: TIMER4_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_COUNT_COUNT_MASK              0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_COUNT_COUNT_SHIFT             0
#define BCHP_LEAP_CTRL_TIMERS_TIMER4_COUNT_COUNT_DEFAULT           0x00000000

/***************************************************************************
 *TIMER5 - Timer5 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER5 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_ENABLE_MASK                   0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_ENABLE_SHIFT                  31
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_ENABLE_DEFAULT                0x00000000

/* LEAP_CTRL_TIMERS :: TIMER5 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_PERIODIC_MASK                 0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_PERIODIC_SHIFT                30
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_PERIODIC_DEFAULT              0x00000000

/* LEAP_CTRL_TIMERS :: TIMER5 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_COMPARE_MASK                  0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_COMPARE_SHIFT                 0
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_COMPARE_DEFAULT               0x00000000

/***************************************************************************
 *TIMER5_COUNT - Timer5 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER5_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_COUNT_reserved0_MASK          0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_COUNT_reserved0_SHIFT         30

/* LEAP_CTRL_TIMERS :: TIMER5_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_COUNT_COUNT_MASK              0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_COUNT_COUNT_SHIFT             0
#define BCHP_LEAP_CTRL_TIMERS_TIMER5_COUNT_COUNT_DEFAULT           0x00000000

/***************************************************************************
 *TIMER6 - Timer6 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER6 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_ENABLE_MASK                   0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_ENABLE_SHIFT                  31
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_ENABLE_DEFAULT                0x00000000

/* LEAP_CTRL_TIMERS :: TIMER6 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_PERIODIC_MASK                 0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_PERIODIC_SHIFT                30
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_PERIODIC_DEFAULT              0x00000000

/* LEAP_CTRL_TIMERS :: TIMER6 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_COMPARE_MASK                  0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_COMPARE_SHIFT                 0
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_COMPARE_DEFAULT               0x00000000

/***************************************************************************
 *TIMER6_COUNT - Timer6 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER6_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_COUNT_reserved0_MASK          0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_COUNT_reserved0_SHIFT         30

/* LEAP_CTRL_TIMERS :: TIMER6_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_COUNT_COUNT_MASK              0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_COUNT_COUNT_SHIFT             0
#define BCHP_LEAP_CTRL_TIMERS_TIMER6_COUNT_COUNT_DEFAULT           0x00000000

/***************************************************************************
 *TIMER7 - Timer7 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER7 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_ENABLE_MASK                   0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_ENABLE_SHIFT                  31
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_ENABLE_DEFAULT                0x00000000

/* LEAP_CTRL_TIMERS :: TIMER7 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_PERIODIC_MASK                 0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_PERIODIC_SHIFT                30
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_PERIODIC_DEFAULT              0x00000000

/* LEAP_CTRL_TIMERS :: TIMER7 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_COMPARE_MASK                  0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_COMPARE_SHIFT                 0
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_COMPARE_DEFAULT               0x00000000

/***************************************************************************
 *TIMER7_COUNT - Timer7 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER7_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_COUNT_reserved0_MASK          0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_COUNT_reserved0_SHIFT         30

/* LEAP_CTRL_TIMERS :: TIMER7_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_COUNT_COUNT_MASK              0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_COUNT_COUNT_SHIFT             0
#define BCHP_LEAP_CTRL_TIMERS_TIMER7_COUNT_COUNT_DEFAULT           0x00000000

/***************************************************************************
 *TIMER8 - Timer8 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER8 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_ENABLE_MASK                   0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_ENABLE_SHIFT                  31
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_ENABLE_DEFAULT                0x00000000

/* LEAP_CTRL_TIMERS :: TIMER8 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_PERIODIC_MASK                 0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_PERIODIC_SHIFT                30
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_PERIODIC_DEFAULT              0x00000000

/* LEAP_CTRL_TIMERS :: TIMER8 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_COMPARE_MASK                  0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_COMPARE_SHIFT                 0
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_COMPARE_DEFAULT               0x00000000

/***************************************************************************
 *TIMER8_COUNT - Timer8 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER8_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_COUNT_reserved0_MASK          0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_COUNT_reserved0_SHIFT         30

/* LEAP_CTRL_TIMERS :: TIMER8_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_COUNT_COUNT_MASK              0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_COUNT_COUNT_SHIFT             0
#define BCHP_LEAP_CTRL_TIMERS_TIMER8_COUNT_COUNT_DEFAULT           0x00000000

/***************************************************************************
 *TIMER9 - Timer9 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER9 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_ENABLE_MASK                   0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_ENABLE_SHIFT                  31
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_ENABLE_DEFAULT                0x00000000

/* LEAP_CTRL_TIMERS :: TIMER9 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_PERIODIC_MASK                 0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_PERIODIC_SHIFT                30
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_PERIODIC_DEFAULT              0x00000000

/* LEAP_CTRL_TIMERS :: TIMER9 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_COMPARE_MASK                  0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_COMPARE_SHIFT                 0
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_COMPARE_DEFAULT               0x00000000

/***************************************************************************
 *TIMER9_COUNT - Timer9 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER9_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_COUNT_reserved0_MASK          0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_COUNT_reserved0_SHIFT         30

/* LEAP_CTRL_TIMERS :: TIMER9_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_COUNT_COUNT_MASK              0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_COUNT_COUNT_SHIFT             0
#define BCHP_LEAP_CTRL_TIMERS_TIMER9_COUNT_COUNT_DEFAULT           0x00000000

/***************************************************************************
 *TIMER10 - Timer10 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER10 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_ENABLE_MASK                  0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_ENABLE_SHIFT                 31
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_ENABLE_DEFAULT               0x00000000

/* LEAP_CTRL_TIMERS :: TIMER10 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_PERIODIC_MASK                0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_PERIODIC_SHIFT               30
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_PERIODIC_DEFAULT             0x00000000

/* LEAP_CTRL_TIMERS :: TIMER10 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_COMPARE_MASK                 0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_COMPARE_SHIFT                0
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_COMPARE_DEFAULT              0x00000000

/***************************************************************************
 *TIMER10_COUNT - Timer10 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER10_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_COUNT_reserved0_MASK         0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_COUNT_reserved0_SHIFT        30

/* LEAP_CTRL_TIMERS :: TIMER10_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_COUNT_COUNT_MASK             0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_COUNT_COUNT_SHIFT            0
#define BCHP_LEAP_CTRL_TIMERS_TIMER10_COUNT_COUNT_DEFAULT          0x00000000

/***************************************************************************
 *TIMER11 - Timer11 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER11 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_ENABLE_MASK                  0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_ENABLE_SHIFT                 31
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_ENABLE_DEFAULT               0x00000000

/* LEAP_CTRL_TIMERS :: TIMER11 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_PERIODIC_MASK                0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_PERIODIC_SHIFT               30
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_PERIODIC_DEFAULT             0x00000000

/* LEAP_CTRL_TIMERS :: TIMER11 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_COMPARE_MASK                 0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_COMPARE_SHIFT                0
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_COMPARE_DEFAULT              0x00000000

/***************************************************************************
 *TIMER11_COUNT - Timer11 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER11_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_COUNT_reserved0_MASK         0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_COUNT_reserved0_SHIFT        30

/* LEAP_CTRL_TIMERS :: TIMER11_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_COUNT_COUNT_MASK             0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_COUNT_COUNT_SHIFT            0
#define BCHP_LEAP_CTRL_TIMERS_TIMER11_COUNT_COUNT_DEFAULT          0x00000000

/***************************************************************************
 *TIMER12 - Timer12 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER12 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_ENABLE_MASK                  0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_ENABLE_SHIFT                 31
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_ENABLE_DEFAULT               0x00000000

/* LEAP_CTRL_TIMERS :: TIMER12 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_PERIODIC_MASK                0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_PERIODIC_SHIFT               30
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_PERIODIC_DEFAULT             0x00000000

/* LEAP_CTRL_TIMERS :: TIMER12 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_COMPARE_MASK                 0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_COMPARE_SHIFT                0
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_COMPARE_DEFAULT              0x00000000

/***************************************************************************
 *TIMER12_COUNT - Timer12 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER12_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_COUNT_reserved0_MASK         0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_COUNT_reserved0_SHIFT        30

/* LEAP_CTRL_TIMERS :: TIMER12_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_COUNT_COUNT_MASK             0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_COUNT_COUNT_SHIFT            0
#define BCHP_LEAP_CTRL_TIMERS_TIMER12_COUNT_COUNT_DEFAULT          0x00000000

/***************************************************************************
 *TIMER13 - Timer13 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER13 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_ENABLE_MASK                  0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_ENABLE_SHIFT                 31
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_ENABLE_DEFAULT               0x00000000

/* LEAP_CTRL_TIMERS :: TIMER13 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_PERIODIC_MASK                0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_PERIODIC_SHIFT               30
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_PERIODIC_DEFAULT             0x00000000

/* LEAP_CTRL_TIMERS :: TIMER13 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_COMPARE_MASK                 0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_COMPARE_SHIFT                0
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_COMPARE_DEFAULT              0x00000000

/***************************************************************************
 *TIMER13_COUNT - Timer13 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER13_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_COUNT_reserved0_MASK         0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_COUNT_reserved0_SHIFT        30

/* LEAP_CTRL_TIMERS :: TIMER13_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_COUNT_COUNT_MASK             0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_COUNT_COUNT_SHIFT            0
#define BCHP_LEAP_CTRL_TIMERS_TIMER13_COUNT_COUNT_DEFAULT          0x00000000

/***************************************************************************
 *TIMER14 - Timer14 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER14 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_ENABLE_MASK                  0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_ENABLE_SHIFT                 31
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_ENABLE_DEFAULT               0x00000000

/* LEAP_CTRL_TIMERS :: TIMER14 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_PERIODIC_MASK                0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_PERIODIC_SHIFT               30
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_PERIODIC_DEFAULT             0x00000000

/* LEAP_CTRL_TIMERS :: TIMER14 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_COMPARE_MASK                 0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_COMPARE_SHIFT                0
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_COMPARE_DEFAULT              0x00000000

/***************************************************************************
 *TIMER14_COUNT - Timer14 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER14_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_COUNT_reserved0_MASK         0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_COUNT_reserved0_SHIFT        30

/* LEAP_CTRL_TIMERS :: TIMER14_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_COUNT_COUNT_MASK             0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_COUNT_COUNT_SHIFT            0
#define BCHP_LEAP_CTRL_TIMERS_TIMER14_COUNT_COUNT_DEFAULT          0x00000000

/***************************************************************************
 *TIMER15 - Timer15 Control Register
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER15 :: ENABLE [31:31] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_ENABLE_MASK                  0x80000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_ENABLE_SHIFT                 31
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_ENABLE_DEFAULT               0x00000000

/* LEAP_CTRL_TIMERS :: TIMER15 :: PERIODIC [30:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_PERIODIC_MASK                0x40000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_PERIODIC_SHIFT               30
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_PERIODIC_DEFAULT             0x00000000

/* LEAP_CTRL_TIMERS :: TIMER15 :: COMPARE [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_COMPARE_MASK                 0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_COMPARE_SHIFT                0
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_COMPARE_DEFAULT              0x00000000

/***************************************************************************
 *TIMER15_COUNT - Timer15 Current Count
 ***************************************************************************/
/* LEAP_CTRL_TIMERS :: TIMER15_COUNT :: reserved0 [31:30] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_COUNT_reserved0_MASK         0xc0000000
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_COUNT_reserved0_SHIFT        30

/* LEAP_CTRL_TIMERS :: TIMER15_COUNT :: COUNT [29:00] */
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_COUNT_COUNT_MASK             0x3fffffff
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_COUNT_COUNT_SHIFT            0
#define BCHP_LEAP_CTRL_TIMERS_TIMER15_COUNT_COUNT_DEFAULT          0x00000000

#endif /* #ifndef BCHP_LEAP_CTRL_TIMERS_H__ */

/* End of File */
