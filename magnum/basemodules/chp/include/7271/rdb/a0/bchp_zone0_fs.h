/****************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Fri Aug 21 14:43:26 2015
 *                 Full Compile MD5 Checksum  6f40c93fa7adf1b7b596c84d59590a10
 *                     (minus title and desc)
 *                 MD5 Checksum               b1b8c76af39c441b8e9ab1ae2930543d
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     88
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_ZONE0_FS_H__
#define BCHP_ZONE0_FS_H__

/***************************************************************************
 *ZONE0_FS
 ***************************************************************************/
#define BCHP_ZONE0_FS_PWR_CONTROL                0x2041d020 /* [RW] Zone Control Register */
#define BCHP_ZONE0_FS_PWR_CONFIG1                0x2041d024 /* [RW] Zone Configuration Register #1 */
#define BCHP_ZONE0_FS_PWR_CONFIG2                0x2041d028 /* [RW] Zone Configuration Register #2 */
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_CTRL       0x2041d030 /* [RW] DPG PWRON Timer Control Register */
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_STATUS     0x2041d034 /* [RO] DPG PWRON Timer Status Register */

/***************************************************************************
 *PWR_CONTROL - Zone Control Register
 ***************************************************************************/
/* ZONE0_FS :: PWR_CONTROL :: ZONE_RESET_STATE [31:31] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_RESET_STATE_MASK            0x80000000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_RESET_STATE_SHIFT           31

/* ZONE0_FS :: PWR_CONTROL :: ZONE_ISO_STATE [30:30] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_ISO_STATE_MASK              0x40000000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_ISO_STATE_SHIFT             30

/* ZONE0_FS :: PWR_CONTROL :: ZONE_MEM_PWR_STATE [29:29] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MEM_PWR_STATE_MASK          0x20000000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MEM_PWR_STATE_SHIFT         29

/* ZONE0_FS :: PWR_CONTROL :: ZONE_DPG_PWR_STATE [28:28] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_DPG_PWR_STATE_MASK          0x10000000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_DPG_PWR_STATE_SHIFT         28

/* ZONE0_FS :: PWR_CONTROL :: ZONE_POWER_GOOD [27:27] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_POWER_GOOD_MASK             0x08000000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_POWER_GOOD_SHIFT            27

/* ZONE0_FS :: PWR_CONTROL :: ZONE_PWR_ON_STATE [26:26] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_ON_STATE_MASK           0x04000000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_ON_STATE_SHIFT          26

/* ZONE0_FS :: PWR_CONTROL :: ZONE_PWR_OFF_STATE [25:25] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_OFF_STATE_MASK          0x02000000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_OFF_STATE_SHIFT         25

/* ZONE0_FS :: PWR_CONTROL :: FREQ_SCALAR_DYNAMIC_SEL [24:24] */
#define BCHP_ZONE0_FS_PWR_CONTROL_FREQ_SCALAR_DYNAMIC_SEL_MASK     0x01000000
#define BCHP_ZONE0_FS_PWR_CONTROL_FREQ_SCALAR_DYNAMIC_SEL_SHIFT    24

/* ZONE0_FS :: PWR_CONTROL :: ZONE_PWR_CNTL_STATE [23:19] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_CNTL_STATE_MASK         0x00f80000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_CNTL_STATE_SHIFT        19

/* ZONE0_FS :: PWR_CONTROL :: PSM_VDD_READBACK [18:17] */
#define BCHP_ZONE0_FS_PWR_CONTROL_PSM_VDD_READBACK_MASK            0x00060000
#define BCHP_ZONE0_FS_PWR_CONTROL_PSM_VDD_READBACK_SHIFT           17

/* ZONE0_FS :: PWR_CONTROL :: reserved0 [16:16] */
#define BCHP_ZONE0_FS_PWR_CONTROL_reserved0_MASK                   0x00010000
#define BCHP_ZONE0_FS_PWR_CONTROL_reserved0_SHIFT                  16

/* ZONE0_FS :: PWR_CONTROL :: ZONE_MAN_DPGN_CNTL [15:15] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_DPGN_CNTL_MASK          0x00008000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_DPGN_CNTL_SHIFT         15
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_DPGN_CNTL_DEFAULT       0x00000000

/* ZONE0_FS :: PWR_CONTROL :: ZONE_MAN_DPG1_CNTL [14:14] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_DPG1_CNTL_MASK          0x00004000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_DPG1_CNTL_SHIFT         14
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_DPG1_CNTL_DEFAULT       0x00000000

/* ZONE0_FS :: PWR_CONTROL :: ZONE_MEM_STBY [13:13] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MEM_STBY_MASK               0x00002000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MEM_STBY_SHIFT              13
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MEM_STBY_DEFAULT            0x00000000

/* ZONE0_FS :: PWR_CONTROL :: ZONE_BLK_RST_ASSERT [12:12] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_BLK_RST_ASSERT_MASK         0x00001000
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_BLK_RST_ASSERT_SHIFT        12
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_BLK_RST_ASSERT_DEFAULT      0x00000001

/* ZONE0_FS :: PWR_CONTROL :: ZONE_MEM_PWR_CNTL_EN [11:11] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MEM_PWR_CNTL_EN_MASK        0x00000800
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MEM_PWR_CNTL_EN_SHIFT       11
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MEM_PWR_CNTL_EN_DEFAULT     0x00000001

/* ZONE0_FS :: PWR_CONTROL :: ZONE_PWR_UP_REQ [10:10] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_UP_REQ_MASK             0x00000400
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_UP_REQ_SHIFT            10
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_UP_REQ_DEFAULT          0x00000000

/* ZONE0_FS :: PWR_CONTROL :: ZONE_PWR_DN_REQ [09:09] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_DN_REQ_MASK             0x00000200
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_DN_REQ_SHIFT            9
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_PWR_DN_REQ_DEFAULT          0x00000000

/* ZONE0_FS :: PWR_CONTROL :: ZONE_DPG_CNTL_EN [08:08] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_DPG_CNTL_EN_MASK            0x00000100
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_DPG_CNTL_EN_SHIFT           8
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_DPG_CNTL_EN_DEFAULT         0x00000001

/* ZONE0_FS :: PWR_CONTROL :: ZONE_MANUAL_CONTROL [07:07] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MANUAL_CONTROL_MASK         0x00000080
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MANUAL_CONTROL_SHIFT        7
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MANUAL_CONTROL_DEFAULT      0x00000000

/* ZONE0_FS :: PWR_CONTROL :: ZONE_MAN_ISO_CNTL [06:06] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_ISO_CNTL_MASK           0x00000040
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_ISO_CNTL_SHIFT          6
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_ISO_CNTL_DEFAULT        0x00000000

/* ZONE0_FS :: PWR_CONTROL :: reserved1 [05:05] */
#define BCHP_ZONE0_FS_PWR_CONTROL_reserved1_MASK                   0x00000020
#define BCHP_ZONE0_FS_PWR_CONTROL_reserved1_SHIFT                  5

/* ZONE0_FS :: PWR_CONTROL :: ZONE_MAN_MEM_PWR [04:04] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_MEM_PWR_MASK            0x00000010
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_MEM_PWR_SHIFT           4
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_MEM_PWR_DEFAULT         0x00000000

/* ZONE0_FS :: PWR_CONTROL :: ZONE_DPG_CAPABLE [03:03] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_DPG_CAPABLE_MASK            0x00000008
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_DPG_CAPABLE_SHIFT           3

/* ZONE0_FS :: PWR_CONTROL :: ZONE_FREQ_SCALE_USED [02:02] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_FREQ_SCALE_USED_MASK        0x00000004
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_FREQ_SCALE_USED_SHIFT       2

/* ZONE0_FS :: PWR_CONTROL :: ZONE_MAN_RESET_CNTL [01:01] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_RESET_CNTL_MASK         0x00000002
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_RESET_CNTL_SHIFT        1
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_RESET_CNTL_DEFAULT      0x00000000

/* ZONE0_FS :: PWR_CONTROL :: ZONE_MAN_CLKEN [00:00] */
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_CLKEN_MASK              0x00000001
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_CLKEN_SHIFT             0
#define BCHP_ZONE0_FS_PWR_CONTROL_ZONE_MAN_CLKEN_DEFAULT           0x00000000

/***************************************************************************
 *PWR_CONFIG1 - Zone Configuration Register #1
 ***************************************************************************/
/* ZONE0_FS :: PWR_CONFIG1 :: RST_OFF_DLY [31:28] */
#define BCHP_ZONE0_FS_PWR_CONFIG1_RST_OFF_DLY_MASK                 0xf0000000
#define BCHP_ZONE0_FS_PWR_CONFIG1_RST_OFF_DLY_SHIFT                28
#define BCHP_ZONE0_FS_PWR_CONFIG1_RST_OFF_DLY_DEFAULT              0x00000004

/* ZONE0_FS :: PWR_CONFIG1 :: RST_ON_DLY [27:24] */
#define BCHP_ZONE0_FS_PWR_CONFIG1_RST_ON_DLY_MASK                  0x0f000000
#define BCHP_ZONE0_FS_PWR_CONFIG1_RST_ON_DLY_SHIFT                 24
#define BCHP_ZONE0_FS_PWR_CONFIG1_RST_ON_DLY_DEFAULT               0x00000004

/* ZONE0_FS :: PWR_CONFIG1 :: CLK_OFF_DLY [23:20] */
#define BCHP_ZONE0_FS_PWR_CONFIG1_CLK_OFF_DLY_MASK                 0x00f00000
#define BCHP_ZONE0_FS_PWR_CONFIG1_CLK_OFF_DLY_SHIFT                20
#define BCHP_ZONE0_FS_PWR_CONFIG1_CLK_OFF_DLY_DEFAULT              0x00000004

/* ZONE0_FS :: PWR_CONFIG1 :: CLK_ON_DLY [19:16] */
#define BCHP_ZONE0_FS_PWR_CONFIG1_CLK_ON_DLY_MASK                  0x000f0000
#define BCHP_ZONE0_FS_PWR_CONFIG1_CLK_ON_DLY_SHIFT                 16
#define BCHP_ZONE0_FS_PWR_CONFIG1_CLK_ON_DLY_DEFAULT               0x00000004

/* ZONE0_FS :: PWR_CONFIG1 :: ISO_OFF_DLY [15:12] */
#define BCHP_ZONE0_FS_PWR_CONFIG1_ISO_OFF_DLY_MASK                 0x0000f000
#define BCHP_ZONE0_FS_PWR_CONFIG1_ISO_OFF_DLY_SHIFT                12
#define BCHP_ZONE0_FS_PWR_CONFIG1_ISO_OFF_DLY_DEFAULT              0x00000004

/* ZONE0_FS :: PWR_CONFIG1 :: ISO_ON_DLY [11:08] */
#define BCHP_ZONE0_FS_PWR_CONFIG1_ISO_ON_DLY_MASK                  0x00000f00
#define BCHP_ZONE0_FS_PWR_CONFIG1_ISO_ON_DLY_SHIFT                 8
#define BCHP_ZONE0_FS_PWR_CONFIG1_ISO_ON_DLY_DEFAULT               0x00000004

/* ZONE0_FS :: PWR_CONFIG1 :: reserved0 [07:05] */
#define BCHP_ZONE0_FS_PWR_CONFIG1_reserved0_MASK                   0x000000e0
#define BCHP_ZONE0_FS_PWR_CONFIG1_reserved0_SHIFT                  5

/* ZONE0_FS :: PWR_CONFIG1 :: PWROK_TRESH [04:03] */
#define BCHP_ZONE0_FS_PWR_CONFIG1_PWROK_TRESH_MASK                 0x00000018
#define BCHP_ZONE0_FS_PWR_CONFIG1_PWROK_TRESH_SHIFT                3
#define BCHP_ZONE0_FS_PWR_CONFIG1_PWROK_TRESH_DEFAULT              0x00000000

/* ZONE0_FS :: PWR_CONFIG1 :: PWROK_DELAY_SEL [02:00] */
#define BCHP_ZONE0_FS_PWR_CONFIG1_PWROK_DELAY_SEL_MASK             0x00000007
#define BCHP_ZONE0_FS_PWR_CONFIG1_PWROK_DELAY_SEL_SHIFT            0
#define BCHP_ZONE0_FS_PWR_CONFIG1_PWROK_DELAY_SEL_DEFAULT          0x00000000

/***************************************************************************
 *PWR_CONFIG2 - Zone Configuration Register #2
 ***************************************************************************/
/* ZONE0_FS :: PWR_CONFIG2 :: MEM_OFF_DLY [31:28] */
#define BCHP_ZONE0_FS_PWR_CONFIG2_MEM_OFF_DLY_MASK                 0xf0000000
#define BCHP_ZONE0_FS_PWR_CONFIG2_MEM_OFF_DLY_SHIFT                28
#define BCHP_ZONE0_FS_PWR_CONFIG2_MEM_OFF_DLY_DEFAULT              0x00000004

/* ZONE0_FS :: PWR_CONFIG2 :: MEM_ON_DLY [27:24] */
#define BCHP_ZONE0_FS_PWR_CONFIG2_MEM_ON_DLY_MASK                  0x0f000000
#define BCHP_ZONE0_FS_PWR_CONFIG2_MEM_ON_DLY_SHIFT                 24
#define BCHP_ZONE0_FS_PWR_CONFIG2_MEM_ON_DLY_DEFAULT               0x00000004

/* ZONE0_FS :: PWR_CONFIG2 :: DPG_OFF_DLY [23:20] */
#define BCHP_ZONE0_FS_PWR_CONFIG2_DPG_OFF_DLY_MASK                 0x00f00000
#define BCHP_ZONE0_FS_PWR_CONFIG2_DPG_OFF_DLY_SHIFT                20
#define BCHP_ZONE0_FS_PWR_CONFIG2_DPG_OFF_DLY_DEFAULT              0x00000004

/* ZONE0_FS :: PWR_CONFIG2 :: DPG1_ON_DLY [19:16] */
#define BCHP_ZONE0_FS_PWR_CONFIG2_DPG1_ON_DLY_MASK                 0x000f0000
#define BCHP_ZONE0_FS_PWR_CONFIG2_DPG1_ON_DLY_SHIFT                16
#define BCHP_ZONE0_FS_PWR_CONFIG2_DPG1_ON_DLY_DEFAULT              0x00000003

/* ZONE0_FS :: PWR_CONFIG2 :: DPGN_ON_DLY [15:12] */
#define BCHP_ZONE0_FS_PWR_CONFIG2_DPGN_ON_DLY_MASK                 0x0000f000
#define BCHP_ZONE0_FS_PWR_CONFIG2_DPGN_ON_DLY_SHIFT                12
#define BCHP_ZONE0_FS_PWR_CONFIG2_DPGN_ON_DLY_DEFAULT              0x00000001

/* ZONE0_FS :: PWR_CONFIG2 :: reserved0 [11:06] */
#define BCHP_ZONE0_FS_PWR_CONFIG2_reserved0_MASK                   0x00000fc0
#define BCHP_ZONE0_FS_PWR_CONFIG2_reserved0_SHIFT                  6

/* ZONE0_FS :: PWR_CONFIG2 :: SLEW_PRESCALE_SEL [05:03] */
#define BCHP_ZONE0_FS_PWR_CONFIG2_SLEW_PRESCALE_SEL_MASK           0x00000038
#define BCHP_ZONE0_FS_PWR_CONFIG2_SLEW_PRESCALE_SEL_SHIFT          3
#define BCHP_ZONE0_FS_PWR_CONFIG2_SLEW_PRESCALE_SEL_DEFAULT        0x00000002

/* ZONE0_FS :: PWR_CONFIG2 :: DELAY_PRESCALE_SEL [02:00] */
#define BCHP_ZONE0_FS_PWR_CONFIG2_DELAY_PRESCALE_SEL_MASK          0x00000007
#define BCHP_ZONE0_FS_PWR_CONFIG2_DELAY_PRESCALE_SEL_SHIFT         0
#define BCHP_ZONE0_FS_PWR_CONFIG2_DELAY_PRESCALE_SEL_DEFAULT       0x00000002

/***************************************************************************
 *DPG_PWRON_TIMER_CTRL - DPG PWRON Timer Control Register
 ***************************************************************************/
/* ZONE0_FS :: DPG_PWRON_TIMER_CTRL :: reserved0 [31:19] */
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_CTRL_reserved0_MASK          0xfff80000
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_CTRL_reserved0_SHIFT         19

/* ZONE0_FS :: DPG_PWRON_TIMER_CTRL :: DPG_PWRON_TIMER_RESET [18:18] */
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_CTRL_DPG_PWRON_TIMER_RESET_MASK 0x00040000
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_CTRL_DPG_PWRON_TIMER_RESET_SHIFT 18
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_CTRL_DPG_PWRON_TIMER_RESET_DEFAULT 0x00000000

/* ZONE0_FS :: DPG_PWRON_TIMER_CTRL :: DPG_PWRON_TIMER_THRESH [17:00] */
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_CTRL_DPG_PWRON_TIMER_THRESH_MASK 0x0003ffff
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_CTRL_DPG_PWRON_TIMER_THRESH_SHIFT 0
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_CTRL_DPG_PWRON_TIMER_THRESH_DEFAULT 0x0000ffff

/***************************************************************************
 *DPG_PWRON_TIMER_STATUS - DPG PWRON Timer Status Register
 ***************************************************************************/
/* ZONE0_FS :: DPG_PWRON_TIMER_STATUS :: reserved0 [31:19] */
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_STATUS_reserved0_MASK        0xfff80000
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_STATUS_reserved0_SHIFT       19

/* ZONE0_FS :: DPG_PWRON_TIMER_STATUS :: DPG_PWRON_TIMEOUT [18:18] */
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_STATUS_DPG_PWRON_TIMEOUT_MASK 0x00040000
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_STATUS_DPG_PWRON_TIMEOUT_SHIFT 18

/* ZONE0_FS :: DPG_PWRON_TIMER_STATUS :: DPG_PWRON_TIMER_VALUE [17:00] */
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_STATUS_DPG_PWRON_TIMER_VALUE_MASK 0x0003ffff
#define BCHP_ZONE0_FS_DPG_PWRON_TIMER_STATUS_DPG_PWRON_TIMER_VALUE_SHIFT 0

#endif /* #ifndef BCHP_ZONE0_FS_H__ */

/* End of File */
