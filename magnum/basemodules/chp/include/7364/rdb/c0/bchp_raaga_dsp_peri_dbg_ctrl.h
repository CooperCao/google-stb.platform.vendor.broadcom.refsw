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
 * Date:           Generated on               Mon Feb  8 12:53:15 2016
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

#ifndef BCHP_RAAGA_DSP_PERI_DBG_CTRL_H__
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_H__

/***************************************************************************
 *RAAGA_DSP_PERI_DBG_CTRL - Raaga DSP Debug Control Registers
 ***************************************************************************/
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS 0x00c21080 /* [RO] DSP Subsystem UART Status */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_RCV_DATA 0x00c21084 /* [RO] DSP Subsystem UART Receive Data */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_XMIT_DATA 0x00c21088 /* [RW] DSP Subsystem UART Transmit Data */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL   0x00c2108c /* [RW] DSP Subsystem UART Control */

/***************************************************************************
 *UART_STATUS - DSP Subsystem UART Status
 ***************************************************************************/
/* RAAGA_DSP_PERI_DBG_CTRL :: UART_STATUS :: reserved0 [31:24] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_reserved0_MASK    0xff000000
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_reserved0_SHIFT   24

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_STATUS :: XMIT_FIFO_AVAIL [23:16] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_XMIT_FIFO_AVAIL_MASK 0x00ff0000
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_XMIT_FIFO_AVAIL_SHIFT 16

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_STATUS :: reserved1 [15:05] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_reserved1_MASK    0x0000ffe0
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_reserved1_SHIFT   5

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_STATUS :: RCV_OVERFLOW [04:04] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_RCV_OVERFLOW_MASK 0x00000010
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_RCV_OVERFLOW_SHIFT 4

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_STATUS :: XMIT_OVERFLOW [03:03] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_XMIT_OVERFLOW_MASK 0x00000008
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_XMIT_OVERFLOW_SHIFT 3

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_STATUS :: XMIT_ACTIVE [02:02] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_XMIT_ACTIVE_MASK  0x00000004
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_XMIT_ACTIVE_SHIFT 2

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_STATUS :: XMIT_FULL [01:01] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_XMIT_FULL_MASK    0x00000002
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_XMIT_FULL_SHIFT   1

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_STATUS :: RCV_EMPTY [00:00] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_RCV_EMPTY_MASK    0x00000001
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS_RCV_EMPTY_SHIFT   0

/***************************************************************************
 *UART_RCV_DATA - DSP Subsystem UART Receive Data
 ***************************************************************************/
/* RAAGA_DSP_PERI_DBG_CTRL :: UART_RCV_DATA :: reserved0 [31:08] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_RCV_DATA_reserved0_MASK  0xffffff00
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_RCV_DATA_reserved0_SHIFT 8

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_RCV_DATA :: RCV_DATA [07:00] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_RCV_DATA_RCV_DATA_MASK   0x000000ff
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_RCV_DATA_RCV_DATA_SHIFT  0

/***************************************************************************
 *UART_XMIT_DATA - DSP Subsystem UART Transmit Data
 ***************************************************************************/
/* RAAGA_DSP_PERI_DBG_CTRL :: UART_XMIT_DATA :: reserved0 [31:08] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_XMIT_DATA_reserved0_MASK 0xffffff00
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_XMIT_DATA_reserved0_SHIFT 8

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_XMIT_DATA :: XMIT_DATA [07:00] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_XMIT_DATA_XMIT_DATA_MASK 0x000000ff
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_XMIT_DATA_XMIT_DATA_SHIFT 0
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_XMIT_DATA_XMIT_DATA_DEFAULT 0x00000000

/***************************************************************************
 *UART_CTRL - DSP Subsystem UART Control
 ***************************************************************************/
/* RAAGA_DSP_PERI_DBG_CTRL :: UART_CTRL :: XMIT_FIFO_LEVEL [31:24] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_XMIT_FIFO_LEVEL_MASK 0xff000000
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_XMIT_FIFO_LEVEL_SHIFT 24
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_XMIT_FIFO_LEVEL_DEFAULT 0x00000000

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_CTRL :: reserved0 [23:18] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_reserved0_MASK      0x00fc0000
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_reserved0_SHIFT     18

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_CTRL :: TX_FIFO_RESET [17:17] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_TX_FIFO_RESET_MASK  0x00020000
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_TX_FIFO_RESET_SHIFT 17
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_TX_FIFO_RESET_DEFAULT 0x00000000

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_CTRL :: ENABLE [16:16] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_ENABLE_MASK         0x00010000
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_ENABLE_SHIFT        16
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_ENABLE_DEFAULT      0x00000000

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_CTRL :: reserved1 [15:12] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_reserved1_MASK      0x0000f000
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_reserved1_SHIFT     12

/* RAAGA_DSP_PERI_DBG_CTRL :: UART_CTRL :: BAUD_RATE_DIV [11:00] */
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_BAUD_RATE_DIV_MASK  0x00000fff
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_BAUD_RATE_DIV_SHIFT 0
#define BCHP_RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL_BAUD_RATE_DIV_DEFAULT 0x00000000

#endif /* #ifndef BCHP_RAAGA_DSP_PERI_DBG_CTRL_H__ */

/* End of File */
