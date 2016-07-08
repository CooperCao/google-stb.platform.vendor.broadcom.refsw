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
 * Date:           Generated on               Thu Jun 18 10:52:56 2015
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

#ifndef BCHP_HDCP2_RX_HAE_INTR2_0_H__
#define BCHP_HDCP2_RX_HAE_INTR2_0_H__

/***************************************************************************
 *HDCP2_RX_HAE_INTR2_0 - HDMI to HAE Level 2 Interrupt Registers
 ***************************************************************************/
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS     0x206f2ac0 /* [RO] HAE interrupt Status Register */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET        0x206f2ac4 /* [WO] HAE interrupt Set Register */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR      0x206f2ac8 /* [WO] HAE interrupt Clear Register */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS 0x206f2acc /* [RO] HAE interrupt Mask Status Register */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET   0x206f2ad0 /* [WO] HAE interrupt Mask Set Register */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR 0x206f2ad4 /* [WO] HAE interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - HAE interrupt Status Register
 ***************************************************************************/
/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: reserved0 [31:14] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_reserved0_MASK        0xffffc000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_reserved0_SHIFT       14

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: AES_FRAME_NUMBER_ROLLOVER [13:13] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_AES_FRAME_NUMBER_ROLLOVER_MASK 0x00002000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_AES_FRAME_NUMBER_ROLLOVER_SHIFT 13
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_AES_FRAME_NUMBER_ROLLOVER_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: EXCESSIVE_PACKET_ERRORS [12:12] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_EXCESSIVE_PACKET_ERRORS_MASK 0x00001000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_EXCESSIVE_PACKET_ERRORS_SHIFT 12
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_EXCESSIVE_PACKET_ERRORS_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: READ_TOO_FEW_BYTES [11:11] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_TOO_FEW_BYTES_MASK 0x00000800
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_TOO_FEW_BYTES_SHIFT 11
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_TOO_FEW_BYTES_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: READ_MESSAGE_START [10:10] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_MESSAGE_START_MASK 0x00000400
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_MESSAGE_START_SHIFT 10
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_MESSAGE_START_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: WRITE_MESSAGE_START [09:09] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_WRITE_MESSAGE_START_MASK 0x00000200
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_WRITE_MESSAGE_START_SHIFT 9
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_WRITE_MESSAGE_START_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: MAILBOX_UPDATE_1 [08:08] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_MAILBOX_UPDATE_1_MASK 0x00000100
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_MAILBOX_UPDATE_1_SHIFT 8
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_MAILBOX_UPDATE_1_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: MAILBOX_UPDATE_0 [07:07] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_MAILBOX_UPDATE_0_MASK 0x00000080
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_MAILBOX_UPDATE_0_SHIFT 7
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_MAILBOX_UPDATE_0_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: WRITE_FIFO_ERR [06:06] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_WRITE_FIFO_ERR_MASK   0x00000040
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_WRITE_FIFO_ERR_SHIFT  6
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_WRITE_FIFO_ERR_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: READ_FIFO_ERR [05:05] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_FIFO_ERR_MASK    0x00000020
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_FIFO_ERR_SHIFT   5
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_FIFO_ERR_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: HDCP2_I2C_ACCESS_ERR [04:04] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_HDCP2_I2C_ACCESS_ERR_MASK 0x00000010
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_HDCP2_I2C_ACCESS_ERR_SHIFT 4
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_HDCP2_I2C_ACCESS_ERR_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: HDCP1_I2C_ACCESS_ERR [03:03] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_HDCP1_I2C_ACCESS_ERR_MASK 0x00000008
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_HDCP1_I2C_ACCESS_ERR_SHIFT 3
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_HDCP1_I2C_ACCESS_ERR_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: READ_MESSAGE_COMPLETE [02:02] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_MESSAGE_COMPLETE_MASK 0x00000004
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_MESSAGE_COMPLETE_SHIFT 2
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_READ_MESSAGE_COMPLETE_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: WRITE_MESSAGE_RDY [01:01] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_WRITE_MESSAGE_RDY_MASK 0x00000002
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_WRITE_MESSAGE_RDY_SHIFT 1
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_WRITE_MESSAGE_RDY_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_STATUS :: TIMEOUT [00:00] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_TIMEOUT_MASK          0x00000001
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_TIMEOUT_SHIFT         0
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_STATUS_TIMEOUT_DEFAULT       0x00000000

/***************************************************************************
 *CPU_SET - HAE interrupt Set Register
 ***************************************************************************/
/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: reserved0 [31:14] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_reserved0_MASK           0xffffc000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_reserved0_SHIFT          14

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: AES_FRAME_NUMBER_ROLLOVER [13:13] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_AES_FRAME_NUMBER_ROLLOVER_MASK 0x00002000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_AES_FRAME_NUMBER_ROLLOVER_SHIFT 13
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_AES_FRAME_NUMBER_ROLLOVER_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: EXCESSIVE_PACKET_ERRORS [12:12] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_EXCESSIVE_PACKET_ERRORS_MASK 0x00001000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_EXCESSIVE_PACKET_ERRORS_SHIFT 12
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_EXCESSIVE_PACKET_ERRORS_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: READ_TOO_FEW_BYTES [11:11] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_TOO_FEW_BYTES_MASK  0x00000800
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_TOO_FEW_BYTES_SHIFT 11
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_TOO_FEW_BYTES_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: READ_MESSAGE_START [10:10] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_MESSAGE_START_MASK  0x00000400
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_MESSAGE_START_SHIFT 10
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_MESSAGE_START_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: WRITE_MESSAGE_START [09:09] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_WRITE_MESSAGE_START_MASK 0x00000200
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_WRITE_MESSAGE_START_SHIFT 9
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_WRITE_MESSAGE_START_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: MAILBOX_UPDATE_1 [08:08] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_MAILBOX_UPDATE_1_MASK    0x00000100
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_MAILBOX_UPDATE_1_SHIFT   8
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_MAILBOX_UPDATE_1_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: MAILBOX_UPDATE_0 [07:07] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_MAILBOX_UPDATE_0_MASK    0x00000080
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_MAILBOX_UPDATE_0_SHIFT   7
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_MAILBOX_UPDATE_0_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: WRITE_FIFO_ERR [06:06] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_WRITE_FIFO_ERR_MASK      0x00000040
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_WRITE_FIFO_ERR_SHIFT     6
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_WRITE_FIFO_ERR_DEFAULT   0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: READ_FIFO_ERR [05:05] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_FIFO_ERR_MASK       0x00000020
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_FIFO_ERR_SHIFT      5
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_FIFO_ERR_DEFAULT    0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: HDCP2_I2C_ACCESS_ERR [04:04] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_HDCP2_I2C_ACCESS_ERR_MASK 0x00000010
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_HDCP2_I2C_ACCESS_ERR_SHIFT 4
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_HDCP2_I2C_ACCESS_ERR_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: HDCP1_I2C_ACCESS_ERR [03:03] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_HDCP1_I2C_ACCESS_ERR_MASK 0x00000008
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_HDCP1_I2C_ACCESS_ERR_SHIFT 3
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_HDCP1_I2C_ACCESS_ERR_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: READ_MESSAGE_COMPLETE [02:02] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_MESSAGE_COMPLETE_MASK 0x00000004
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_MESSAGE_COMPLETE_SHIFT 2
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_READ_MESSAGE_COMPLETE_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: WRITE_MESSAGE_RDY [01:01] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_WRITE_MESSAGE_RDY_MASK   0x00000002
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_WRITE_MESSAGE_RDY_SHIFT  1
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_WRITE_MESSAGE_RDY_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_SET :: TIMEOUT [00:00] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_TIMEOUT_MASK             0x00000001
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_TIMEOUT_SHIFT            0
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_SET_TIMEOUT_DEFAULT          0x00000000

/***************************************************************************
 *CPU_CLEAR - HAE interrupt Clear Register
 ***************************************************************************/
/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: reserved0 [31:14] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_reserved0_MASK         0xffffc000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_reserved0_SHIFT        14

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: AES_FRAME_NUMBER_ROLLOVER [13:13] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_AES_FRAME_NUMBER_ROLLOVER_MASK 0x00002000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_AES_FRAME_NUMBER_ROLLOVER_SHIFT 13
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_AES_FRAME_NUMBER_ROLLOVER_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: EXCESSIVE_PACKET_ERRORS [12:12] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_EXCESSIVE_PACKET_ERRORS_MASK 0x00001000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_EXCESSIVE_PACKET_ERRORS_SHIFT 12
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_EXCESSIVE_PACKET_ERRORS_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: READ_TOO_FEW_BYTES [11:11] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_TOO_FEW_BYTES_MASK 0x00000800
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_TOO_FEW_BYTES_SHIFT 11
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_TOO_FEW_BYTES_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: READ_MESSAGE_START [10:10] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_MESSAGE_START_MASK 0x00000400
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_MESSAGE_START_SHIFT 10
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_MESSAGE_START_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: WRITE_MESSAGE_START [09:09] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_WRITE_MESSAGE_START_MASK 0x00000200
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_WRITE_MESSAGE_START_SHIFT 9
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_WRITE_MESSAGE_START_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: MAILBOX_UPDATE_1 [08:08] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_MAILBOX_UPDATE_1_MASK  0x00000100
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_MAILBOX_UPDATE_1_SHIFT 8
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_MAILBOX_UPDATE_1_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: MAILBOX_UPDATE_0 [07:07] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_MAILBOX_UPDATE_0_MASK  0x00000080
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_MAILBOX_UPDATE_0_SHIFT 7
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_MAILBOX_UPDATE_0_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: WRITE_FIFO_ERR [06:06] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_WRITE_FIFO_ERR_MASK    0x00000040
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_WRITE_FIFO_ERR_SHIFT   6
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_WRITE_FIFO_ERR_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: READ_FIFO_ERR [05:05] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_FIFO_ERR_MASK     0x00000020
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_FIFO_ERR_SHIFT    5
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_FIFO_ERR_DEFAULT  0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: HDCP2_I2C_ACCESS_ERR [04:04] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_HDCP2_I2C_ACCESS_ERR_MASK 0x00000010
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_HDCP2_I2C_ACCESS_ERR_SHIFT 4
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_HDCP2_I2C_ACCESS_ERR_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: HDCP1_I2C_ACCESS_ERR [03:03] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_HDCP1_I2C_ACCESS_ERR_MASK 0x00000008
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_HDCP1_I2C_ACCESS_ERR_SHIFT 3
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_HDCP1_I2C_ACCESS_ERR_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: READ_MESSAGE_COMPLETE [02:02] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_MESSAGE_COMPLETE_MASK 0x00000004
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_MESSAGE_COMPLETE_SHIFT 2
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_READ_MESSAGE_COMPLETE_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: WRITE_MESSAGE_RDY [01:01] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_WRITE_MESSAGE_RDY_MASK 0x00000002
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_WRITE_MESSAGE_RDY_SHIFT 1
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_WRITE_MESSAGE_RDY_DEFAULT 0x00000000

/* HDCP2_RX_HAE_INTR2_0 :: CPU_CLEAR :: TIMEOUT [00:00] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_TIMEOUT_MASK           0x00000001
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_TIMEOUT_SHIFT          0
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_CLEAR_TIMEOUT_DEFAULT        0x00000000

/***************************************************************************
 *CPU_MASK_STATUS - HAE interrupt Mask Status Register
 ***************************************************************************/
/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: reserved0 [31:14] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_reserved0_MASK   0xffffc000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_reserved0_SHIFT  14

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: AES_FRAME_NUMBER_ROLLOVER [13:13] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_AES_FRAME_NUMBER_ROLLOVER_MASK 0x00002000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_AES_FRAME_NUMBER_ROLLOVER_SHIFT 13
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_AES_FRAME_NUMBER_ROLLOVER_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: EXCESSIVE_PACKET_ERRORS [12:12] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_EXCESSIVE_PACKET_ERRORS_MASK 0x00001000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_EXCESSIVE_PACKET_ERRORS_SHIFT 12
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_EXCESSIVE_PACKET_ERRORS_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: READ_TOO_FEW_BYTES [11:11] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_TOO_FEW_BYTES_MASK 0x00000800
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_TOO_FEW_BYTES_SHIFT 11
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_TOO_FEW_BYTES_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: READ_MESSAGE_START [10:10] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_MESSAGE_START_MASK 0x00000400
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_MESSAGE_START_SHIFT 10
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_MESSAGE_START_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: WRITE_MESSAGE_START [09:09] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_WRITE_MESSAGE_START_MASK 0x00000200
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_WRITE_MESSAGE_START_SHIFT 9
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_WRITE_MESSAGE_START_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: MAILBOX_UPDATE_1 [08:08] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_MAILBOX_UPDATE_1_MASK 0x00000100
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_MAILBOX_UPDATE_1_SHIFT 8
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_MAILBOX_UPDATE_1_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: MAILBOX_UPDATE_0 [07:07] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_MAILBOX_UPDATE_0_MASK 0x00000080
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_MAILBOX_UPDATE_0_SHIFT 7
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_MAILBOX_UPDATE_0_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: WRITE_FIFO_ERR [06:06] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_WRITE_FIFO_ERR_MASK 0x00000040
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_WRITE_FIFO_ERR_SHIFT 6
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_WRITE_FIFO_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: READ_FIFO_ERR [05:05] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_FIFO_ERR_MASK 0x00000020
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_FIFO_ERR_SHIFT 5
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_FIFO_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: HDCP2_I2C_ACCESS_ERR [04:04] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_HDCP2_I2C_ACCESS_ERR_MASK 0x00000010
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_HDCP2_I2C_ACCESS_ERR_SHIFT 4
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_HDCP2_I2C_ACCESS_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: HDCP1_I2C_ACCESS_ERR [03:03] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_HDCP1_I2C_ACCESS_ERR_MASK 0x00000008
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_HDCP1_I2C_ACCESS_ERR_SHIFT 3
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_HDCP1_I2C_ACCESS_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: READ_MESSAGE_COMPLETE [02:02] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_MESSAGE_COMPLETE_MASK 0x00000004
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_MESSAGE_COMPLETE_SHIFT 2
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_READ_MESSAGE_COMPLETE_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: WRITE_MESSAGE_RDY [01:01] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_WRITE_MESSAGE_RDY_MASK 0x00000002
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_WRITE_MESSAGE_RDY_SHIFT 1
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_WRITE_MESSAGE_RDY_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_STATUS :: TIMEOUT [00:00] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_TIMEOUT_MASK     0x00000001
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_TIMEOUT_SHIFT    0
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_STATUS_TIMEOUT_DEFAULT  0x00000001

/***************************************************************************
 *CPU_MASK_SET - HAE interrupt Mask Set Register
 ***************************************************************************/
/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: reserved0 [31:14] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_reserved0_MASK      0xffffc000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_reserved0_SHIFT     14

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: AES_FRAME_NUMBER_ROLLOVER [13:13] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_AES_FRAME_NUMBER_ROLLOVER_MASK 0x00002000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_AES_FRAME_NUMBER_ROLLOVER_SHIFT 13
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_AES_FRAME_NUMBER_ROLLOVER_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: EXCESSIVE_PACKET_ERRORS [12:12] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_EXCESSIVE_PACKET_ERRORS_MASK 0x00001000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_EXCESSIVE_PACKET_ERRORS_SHIFT 12
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_EXCESSIVE_PACKET_ERRORS_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: READ_TOO_FEW_BYTES [11:11] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_TOO_FEW_BYTES_MASK 0x00000800
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_TOO_FEW_BYTES_SHIFT 11
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_TOO_FEW_BYTES_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: READ_MESSAGE_START [10:10] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_MESSAGE_START_MASK 0x00000400
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_MESSAGE_START_SHIFT 10
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_MESSAGE_START_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: WRITE_MESSAGE_START [09:09] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_WRITE_MESSAGE_START_MASK 0x00000200
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_WRITE_MESSAGE_START_SHIFT 9
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_WRITE_MESSAGE_START_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: MAILBOX_UPDATE_1 [08:08] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_MAILBOX_UPDATE_1_MASK 0x00000100
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_MAILBOX_UPDATE_1_SHIFT 8
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_MAILBOX_UPDATE_1_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: MAILBOX_UPDATE_0 [07:07] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_MAILBOX_UPDATE_0_MASK 0x00000080
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_MAILBOX_UPDATE_0_SHIFT 7
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_MAILBOX_UPDATE_0_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: WRITE_FIFO_ERR [06:06] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_WRITE_FIFO_ERR_MASK 0x00000040
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_WRITE_FIFO_ERR_SHIFT 6
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_WRITE_FIFO_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: READ_FIFO_ERR [05:05] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_FIFO_ERR_MASK  0x00000020
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_FIFO_ERR_SHIFT 5
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_FIFO_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: HDCP2_I2C_ACCESS_ERR [04:04] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_HDCP2_I2C_ACCESS_ERR_MASK 0x00000010
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_HDCP2_I2C_ACCESS_ERR_SHIFT 4
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_HDCP2_I2C_ACCESS_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: HDCP1_I2C_ACCESS_ERR [03:03] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_HDCP1_I2C_ACCESS_ERR_MASK 0x00000008
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_HDCP1_I2C_ACCESS_ERR_SHIFT 3
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_HDCP1_I2C_ACCESS_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: READ_MESSAGE_COMPLETE [02:02] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_MESSAGE_COMPLETE_MASK 0x00000004
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_MESSAGE_COMPLETE_SHIFT 2
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_READ_MESSAGE_COMPLETE_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: WRITE_MESSAGE_RDY [01:01] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_WRITE_MESSAGE_RDY_MASK 0x00000002
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_WRITE_MESSAGE_RDY_SHIFT 1
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_WRITE_MESSAGE_RDY_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_SET :: TIMEOUT [00:00] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_TIMEOUT_MASK        0x00000001
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_TIMEOUT_SHIFT       0
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_SET_TIMEOUT_DEFAULT     0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR - HAE interrupt Mask Clear Register
 ***************************************************************************/
/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: reserved0 [31:14] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_reserved0_MASK    0xffffc000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_reserved0_SHIFT   14

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: AES_FRAME_NUMBER_ROLLOVER [13:13] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_AES_FRAME_NUMBER_ROLLOVER_MASK 0x00002000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_AES_FRAME_NUMBER_ROLLOVER_SHIFT 13
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_AES_FRAME_NUMBER_ROLLOVER_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: EXCESSIVE_PACKET_ERRORS [12:12] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_EXCESSIVE_PACKET_ERRORS_MASK 0x00001000
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_EXCESSIVE_PACKET_ERRORS_SHIFT 12
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_EXCESSIVE_PACKET_ERRORS_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: READ_TOO_FEW_BYTES [11:11] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_TOO_FEW_BYTES_MASK 0x00000800
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_TOO_FEW_BYTES_SHIFT 11
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_TOO_FEW_BYTES_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: READ_MESSAGE_START [10:10] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_MESSAGE_START_MASK 0x00000400
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_MESSAGE_START_SHIFT 10
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_MESSAGE_START_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: WRITE_MESSAGE_START [09:09] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_WRITE_MESSAGE_START_MASK 0x00000200
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_WRITE_MESSAGE_START_SHIFT 9
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_WRITE_MESSAGE_START_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: MAILBOX_UPDATE_1 [08:08] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_MAILBOX_UPDATE_1_MASK 0x00000100
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_MAILBOX_UPDATE_1_SHIFT 8
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_MAILBOX_UPDATE_1_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: MAILBOX_UPDATE_0 [07:07] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_MAILBOX_UPDATE_0_MASK 0x00000080
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_MAILBOX_UPDATE_0_SHIFT 7
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_MAILBOX_UPDATE_0_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: WRITE_FIFO_ERR [06:06] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_WRITE_FIFO_ERR_MASK 0x00000040
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_WRITE_FIFO_ERR_SHIFT 6
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_WRITE_FIFO_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: READ_FIFO_ERR [05:05] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_FIFO_ERR_MASK 0x00000020
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_FIFO_ERR_SHIFT 5
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_FIFO_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: HDCP2_I2C_ACCESS_ERR [04:04] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_HDCP2_I2C_ACCESS_ERR_MASK 0x00000010
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_HDCP2_I2C_ACCESS_ERR_SHIFT 4
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_HDCP2_I2C_ACCESS_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: HDCP1_I2C_ACCESS_ERR [03:03] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_HDCP1_I2C_ACCESS_ERR_MASK 0x00000008
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_HDCP1_I2C_ACCESS_ERR_SHIFT 3
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_HDCP1_I2C_ACCESS_ERR_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: READ_MESSAGE_COMPLETE [02:02] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_MESSAGE_COMPLETE_MASK 0x00000004
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_MESSAGE_COMPLETE_SHIFT 2
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_READ_MESSAGE_COMPLETE_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: WRITE_MESSAGE_RDY [01:01] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_WRITE_MESSAGE_RDY_MASK 0x00000002
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_WRITE_MESSAGE_RDY_SHIFT 1
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_WRITE_MESSAGE_RDY_DEFAULT 0x00000001

/* HDCP2_RX_HAE_INTR2_0 :: CPU_MASK_CLEAR :: TIMEOUT [00:00] */
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_TIMEOUT_MASK      0x00000001
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_TIMEOUT_SHIFT     0
#define BCHP_HDCP2_RX_HAE_INTR2_0_CPU_MASK_CLEAR_TIMEOUT_DEFAULT   0x00000001

#endif /* #ifndef BCHP_HDCP2_RX_HAE_INTR2_0_H__ */

/* End of File */
