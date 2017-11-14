/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 *****************************************************************************/
#ifndef BCHP_LEAP_CTRL_MISC_H__
#define BCHP_LEAP_CTRL_MISC_H__

/***************************************************************************
 *LEAP_CTRL_MISC - Misc Control registers
 ***************************************************************************/
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT         0x00100a00 /* [RW][32] Host Access Buffer Request Status */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_SET          0x00100a04 /* [RW][32] Host Access Buffer Request Set */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_CLR          0x00100a08 /* [RW][32] Host Access Buffer Request Clear */
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_STAT        0x00100a0c /* [RW][32] Host Access Buffer Done Status */
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_SET         0x00100a10 /* [RW][32] Host Access Buffer Done Set */
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_CLR         0x00100a14 /* [RW][32] Host Access Buffer Done Clear */
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR             0x00100a18 /* [RO][32] HAB Request/Done Counters */
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR         0x00100a1c /* [RW][32] HAB Request Done Counter Clear */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT0        0x00100a20 /* [RW][32] Host Access Buffer Request Status0 */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT1        0x00100a24 /* [RW][32] Host Access Buffer Request Status1 */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT2        0x00100a28 /* [RW][32] Host Access Buffer Request Status2 */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_PUSH_DATA  0x00100a30 /* [RW][32] Mailbox FIFO Write Data */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_POP_DATA   0x00100a34 /* [RW][32] Mailbox FIFO read Data */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_DEPTH      0x00100a38 /* [RO][32] Mailbox FIFO Current Depth */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_RST_PTRS   0x00100a3c /* [RW][32] Mailbox FIFO Reset Pointers */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_CTRL       0x00100a40 /* [RW][32] Mailbox FIFO Control */
#define BCHP_LEAP_CTRL_MISC_TRACE_Q_DATA         0x00100a44 /* [RW][32] Trace Queue Write/Read Data */
#define BCHP_LEAP_CTRL_MISC_TRACE_Q_RST_PTRS     0x00100a48 /* [RW][32] Trace Queue Reset Pointers */
#define BCHP_LEAP_CTRL_MISC_HW_SEM               0x00100a4c /* [RW][32] General Purpose HW Semaphore */

/***************************************************************************
 *HAB_REQ_STAT - Host Access Buffer Request Status
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_REQ_STAT :: SW_REQ_STAT [31:02] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT_SW_REQ_STAT_MASK          0xfffffffc
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT_SW_REQ_STAT_SHIFT         2
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT_SW_REQ_STAT_DEFAULT       0x00000000

/* LEAP_CTRL_MISC :: HAB_REQ_STAT :: HAB_RESET_REQ_STAT [01:01] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT_HAB_RESET_REQ_STAT_MASK   0x00000002
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT_HAB_RESET_REQ_STAT_SHIFT  1
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT_HAB_RESET_REQ_STAT_DEFAULT 0x00000000

/* LEAP_CTRL_MISC :: HAB_REQ_STAT :: HAB_REQ_STAT [00:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT_HAB_REQ_STAT_MASK         0x00000001
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT_HAB_REQ_STAT_SHIFT        0
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT_HAB_REQ_STAT_DEFAULT      0x00000000

/***************************************************************************
 *HAB_REQ_SET - Host Access Buffer Request Set
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_REQ_SET :: SW_REQ_SET [31:02] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_SET_SW_REQ_SET_MASK            0xfffffffc
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_SET_SW_REQ_SET_SHIFT           2
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_SET_SW_REQ_SET_DEFAULT         0x00000000

/* LEAP_CTRL_MISC :: HAB_REQ_SET :: HAB_RESET_REQ_SET [01:01] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_SET_HAB_RESET_REQ_SET_MASK     0x00000002
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_SET_HAB_RESET_REQ_SET_SHIFT    1
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_SET_HAB_RESET_REQ_SET_DEFAULT  0x00000000

/* LEAP_CTRL_MISC :: HAB_REQ_SET :: HAB_REQ_SET [00:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_SET_HAB_REQ_SET_MASK           0x00000001
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_SET_HAB_REQ_SET_SHIFT          0
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_SET_HAB_REQ_SET_DEFAULT        0x00000000

/***************************************************************************
 *HAB_REQ_CLR - Host Access Buffer Request Clear
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_REQ_CLR :: SW_REQ_CLR [31:02] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_CLR_SW_REQ_CLR_MASK            0xfffffffc
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_CLR_SW_REQ_CLR_SHIFT           2
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_CLR_SW_REQ_CLR_DEFAULT         0x00000000

/* LEAP_CTRL_MISC :: HAB_REQ_CLR :: HAB_RESET_REQ_CLR [01:01] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_CLR_HAB_RESET_REQ_CLR_MASK     0x00000002
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_CLR_HAB_RESET_REQ_CLR_SHIFT    1
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_CLR_HAB_RESET_REQ_CLR_DEFAULT  0x00000000

/* LEAP_CTRL_MISC :: HAB_REQ_CLR :: HAB_REQ_CLR [00:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_CLR_HAB_REQ_CLR_MASK           0x00000001
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_CLR_HAB_REQ_CLR_SHIFT          0
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_CLR_HAB_REQ_CLR_DEFAULT        0x00000000

/***************************************************************************
 *HAB_DONE_STAT - Host Access Buffer Done Status
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_DONE_STAT :: SW_DONE_STAT [31:01] */
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_STAT_SW_DONE_STAT_MASK        0xfffffffe
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_STAT_SW_DONE_STAT_SHIFT       1
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_STAT_SW_DONE_STAT_DEFAULT     0x00000000

/* LEAP_CTRL_MISC :: HAB_DONE_STAT :: HAB_DONE_STAT [00:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_STAT_HAB_DONE_STAT_MASK       0x00000001
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_STAT_HAB_DONE_STAT_SHIFT      0
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_STAT_HAB_DONE_STAT_DEFAULT    0x00000000

/***************************************************************************
 *HAB_DONE_SET - Host Access Buffer Done Set
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_DONE_SET :: SW_DONE_SET [31:01] */
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_SET_SW_DONE_SET_MASK          0xfffffffe
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_SET_SW_DONE_SET_SHIFT         1
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_SET_SW_DONE_SET_DEFAULT       0x00000000

/* LEAP_CTRL_MISC :: HAB_DONE_SET :: HAB_DONE_SET [00:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_SET_HAB_DONE_SET_MASK         0x00000001
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_SET_HAB_DONE_SET_SHIFT        0
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_SET_HAB_DONE_SET_DEFAULT      0x00000000

/***************************************************************************
 *HAB_DONE_CLR - Host Access Buffer Done Clear
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_DONE_CLR :: SW_DONE_CLR [31:01] */
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_CLR_SW_DONE_CLR_MASK          0xfffffffe
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_CLR_SW_DONE_CLR_SHIFT         1
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_CLR_SW_DONE_CLR_DEFAULT       0x00000000

/* LEAP_CTRL_MISC :: HAB_DONE_CLR :: HAB_DONE_CLR [00:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_CLR_HAB_DONE_CLR_MASK         0x00000001
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_CLR_HAB_DONE_CLR_SHIFT        0
#define BCHP_LEAP_CTRL_MISC_HAB_DONE_CLR_HAB_DONE_CLR_DEFAULT      0x00000000

/***************************************************************************
 *HAB_CNTR - HAB Request/Done Counters
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_CNTR :: DONE [31:16] */
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_DONE_MASK                     0xffff0000
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_DONE_SHIFT                    16

/* LEAP_CTRL_MISC :: HAB_CNTR :: REQ [15:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_REQ_MASK                      0x0000ffff
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_REQ_SHIFT                     0

/***************************************************************************
 *HAB_CNTR_CLR - HAB Request Done Counter Clear
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_CNTR_CLR :: reserved0 [31:09] */
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR_reserved0_MASK            0xfffffe00
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR_reserved0_SHIFT           9

/* LEAP_CTRL_MISC :: HAB_CNTR_CLR :: HAB_DONE_CNTR_CLR [08:08] */
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR_HAB_DONE_CNTR_CLR_MASK    0x00000100
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR_HAB_DONE_CNTR_CLR_SHIFT   8
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR_HAB_DONE_CNTR_CLR_DEFAULT 0x00000000

/* LEAP_CTRL_MISC :: HAB_CNTR_CLR :: reserved1 [07:01] */
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR_reserved1_MASK            0x000000fe
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR_reserved1_SHIFT           1

/* LEAP_CTRL_MISC :: HAB_CNTR_CLR :: HAB_REQ_CNTR_CLR [00:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR_HAB_REQ_CNTR_CLR_MASK     0x00000001
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR_HAB_REQ_CNTR_CLR_SHIFT    0
#define BCHP_LEAP_CTRL_MISC_HAB_CNTR_CLR_HAB_REQ_CNTR_CLR_DEFAULT  0x00000000

/***************************************************************************
 *HAB_REQ_STAT0 - Host Access Buffer Request Status0
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_REQ_STAT0 :: SW_REQ_STAT [31:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT0_SW_REQ_STAT_MASK         0xffffffff
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT0_SW_REQ_STAT_SHIFT        0
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT0_SW_REQ_STAT_DEFAULT      0x00000000

/***************************************************************************
 *HAB_REQ_STAT1 - Host Access Buffer Request Status1
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_REQ_STAT1 :: SW_REQ_STAT [31:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT1_SW_REQ_STAT_MASK         0xffffffff
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT1_SW_REQ_STAT_SHIFT        0
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT1_SW_REQ_STAT_DEFAULT      0x00000000

/***************************************************************************
 *HAB_REQ_STAT2 - Host Access Buffer Request Status2
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HAB_REQ_STAT2 :: SW_REQ_STAT [31:00] */
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT2_SW_REQ_STAT_MASK         0xffffffff
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT2_SW_REQ_STAT_SHIFT        0
#define BCHP_LEAP_CTRL_MISC_HAB_REQ_STAT2_SW_REQ_STAT_DEFAULT      0x00000000

/***************************************************************************
 *MBOX_FIFO_PUSH_DATA - Mailbox FIFO Write Data
 ***************************************************************************/
/* LEAP_CTRL_MISC :: MBOX_FIFO_PUSH_DATA :: DATA [31:00] */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_PUSH_DATA_DATA_MASK          0xffffffff
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_PUSH_DATA_DATA_SHIFT         0
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_PUSH_DATA_DATA_DEFAULT       0x00000000

/***************************************************************************
 *MBOX_FIFO_POP_DATA - Mailbox FIFO read Data
 ***************************************************************************/
/* LEAP_CTRL_MISC :: MBOX_FIFO_POP_DATA :: DATA [31:00] */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_POP_DATA_DATA_MASK           0xffffffff
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_POP_DATA_DATA_SHIFT          0
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_POP_DATA_DATA_DEFAULT        0x00000000

/***************************************************************************
 *MBOX_FIFO_DEPTH - Mailbox FIFO Current Depth
 ***************************************************************************/
/* LEAP_CTRL_MISC :: MBOX_FIFO_DEPTH :: reserved0 [31:09] */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_DEPTH_reserved0_MASK         0xfffffe00
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_DEPTH_reserved0_SHIFT        9

/* LEAP_CTRL_MISC :: MBOX_FIFO_DEPTH :: DEPTH [08:00] */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_DEPTH_DEPTH_MASK             0x000001ff
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_DEPTH_DEPTH_SHIFT            0

/***************************************************************************
 *MBOX_FIFO_RST_PTRS - Mailbox FIFO Reset Pointers
 ***************************************************************************/
/* LEAP_CTRL_MISC :: MBOX_FIFO_RST_PTRS :: reserved0 [31:01] */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_RST_PTRS_reserved0_MASK      0xfffffffe
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_RST_PTRS_reserved0_SHIFT     1

/* LEAP_CTRL_MISC :: MBOX_FIFO_RST_PTRS :: RST_PTR [00:00] */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_RST_PTRS_RST_PTR_MASK        0x00000001
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_RST_PTRS_RST_PTR_SHIFT       0
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_RST_PTRS_RST_PTR_DEFAULT     0x00000000

/***************************************************************************
 *MBOX_FIFO_CTRL - Mailbox FIFO Control
 ***************************************************************************/
/* LEAP_CTRL_MISC :: MBOX_FIFO_CTRL :: reserved0 [31:01] */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_CTRL_reserved0_MASK          0xfffffffe
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_CTRL_reserved0_SHIFT         1

/* LEAP_CTRL_MISC :: MBOX_FIFO_CTRL :: POP_CTRL [00:00] */
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_CTRL_POP_CTRL_MASK           0x00000001
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_CTRL_POP_CTRL_SHIFT          0
#define BCHP_LEAP_CTRL_MISC_MBOX_FIFO_CTRL_POP_CTRL_DEFAULT        0x00000000

/***************************************************************************
 *TRACE_Q_DATA - Trace Queue Write/Read Data
 ***************************************************************************/
/* LEAP_CTRL_MISC :: TRACE_Q_DATA :: DATA [31:00] */
#define BCHP_LEAP_CTRL_MISC_TRACE_Q_DATA_DATA_MASK                 0xffffffff
#define BCHP_LEAP_CTRL_MISC_TRACE_Q_DATA_DATA_SHIFT                0
#define BCHP_LEAP_CTRL_MISC_TRACE_Q_DATA_DATA_DEFAULT              0x00000000

/***************************************************************************
 *TRACE_Q_RST_PTRS - Trace Queue Reset Pointers
 ***************************************************************************/
/* LEAP_CTRL_MISC :: TRACE_Q_RST_PTRS :: reserved0 [31:01] */
#define BCHP_LEAP_CTRL_MISC_TRACE_Q_RST_PTRS_reserved0_MASK        0xfffffffe
#define BCHP_LEAP_CTRL_MISC_TRACE_Q_RST_PTRS_reserved0_SHIFT       1

/* LEAP_CTRL_MISC :: TRACE_Q_RST_PTRS :: RST_PTR [00:00] */
#define BCHP_LEAP_CTRL_MISC_TRACE_Q_RST_PTRS_RST_PTR_MASK          0x00000001
#define BCHP_LEAP_CTRL_MISC_TRACE_Q_RST_PTRS_RST_PTR_SHIFT         0
#define BCHP_LEAP_CTRL_MISC_TRACE_Q_RST_PTRS_RST_PTR_DEFAULT       0x00000000

/***************************************************************************
 *HW_SEM - General Purpose HW Semaphore
 ***************************************************************************/
/* LEAP_CTRL_MISC :: HW_SEM :: reserved0 [31:01] */
#define BCHP_LEAP_CTRL_MISC_HW_SEM_reserved0_MASK                  0xfffffffe
#define BCHP_LEAP_CTRL_MISC_HW_SEM_reserved0_SHIFT                 1

/* LEAP_CTRL_MISC :: HW_SEM :: HW_SEM [00:00] */
#define BCHP_LEAP_CTRL_MISC_HW_SEM_HW_SEM_MASK                     0x00000001
#define BCHP_LEAP_CTRL_MISC_HW_SEM_HW_SEM_SHIFT                    0
#define BCHP_LEAP_CTRL_MISC_HW_SEM_HW_SEM_DEFAULT                  0x00000000

#endif /* #ifndef BCHP_LEAP_CTRL_MISC_H__ */

/* End of File */
