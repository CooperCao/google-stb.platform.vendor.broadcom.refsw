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

#ifndef BCHP_XPT_TSIO_CONFIG_REGISTERS_H__
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_H__

/***************************************************************************
 *XPT_TSIO_CONFIG_REGISTERS
 ***************************************************************************/
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL 0x00a0d000 /* [RW] TSIO Control Register */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CLK_SELECT 0x00a0d004 /* [RW] TSIO Serial Clock Select */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_STUFF_PKT_SERVICE_ID 0x00a0d008 /* [RW] TSIO Stuffing Packet ServiceID */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL 0x00a0d010 /* [RW] Transmit C&C Ping Buffer Ctrl */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL 0x00a0d014 /* [RW] Transmit C&C Pong Buffer Ctrl */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS 0x00a0d018 /* [RW] Received C&C Ping Buffer Status */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS 0x00a0d01c /* [RW] Received C&C Pong Buffer Status */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS 0x00a0d02c /* [RW] Received C&C Error Status */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CTS_CHECK_EN 0x00a0d030 /* [RW] Enable usage of the received CTS values during pipe arbitration */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CTS_STATUS 0x00a0d034 /* [RO] Received CTS status from the smartcard */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS 0x00a0d048 /* [RW] Tx Packet drop status per pipe */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CLK_SEL_VALUE 0x00a0d04c /* [RO] TSIO Clk select final value as realised by h/w */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TV_STATUS 0x00a0d050 /* [RO] TV status register */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_0 0x00a0d054 /* [RO] TSIO Debug Register */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_1 0x00a0d058 /* [RO] TSIO Debug Register */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_2 0x00a0d05c /* [RO] TSIO Debug Register */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_3 0x00a0d060 /* [RO] TSIO Debug Register */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_4 0x00a0d064 /* [RO] TSIO Debug Register */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_5 0x00a0d068 /* [RO] TSIO Debug Register */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_6 0x00a0d06c /* [RO] TSIO Debug Register */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_7 0x00a0d070 /* [RO] TSIO Debug Register */

/***************************************************************************
 *SERVICE_ID_TABLE%i - Service ID Table
 ***************************************************************************/
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_SERVICE_ID_TABLEi_ARRAY_BASE 0x00a0c000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_SERVICE_ID_TABLEi_ARRAY_START 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_SERVICE_ID_TABLEi_ARRAY_END 767
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_SERVICE_ID_TABLEi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *SERVICE_ID_TABLE%i - Service ID Table
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: SERVICE_ID_TABLEi :: reserved0 [31:06] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_SERVICE_ID_TABLEi_reserved0_MASK 0xffffffc0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_SERVICE_ID_TABLEi_reserved0_SHIFT 6

/* XPT_TSIO_CONFIG_REGISTERS :: SERVICE_ID_TABLEi :: SERVICE_ID [05:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_SERVICE_ID_TABLEi_SERVICE_ID_MASK 0x0000003f
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_SERVICE_ID_TABLEi_SERVICE_ID_SHIFT 0


/***************************************************************************
 *TSIO_CTRL - TSIO Control Register
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: reserved0 [31:19] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_reserved0_MASK    0xfff80000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_reserved0_SHIFT   19

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: CRXPT_872_DIS [18:18] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_CRXPT_872_DIS_MASK 0x00040000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_CRXPT_872_DIS_SHIFT 18
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_CRXPT_872_DIS_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: SKIP_CC_CHECK_SC_GEN_PKT [17:17] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_SKIP_CC_CHECK_SC_GEN_PKT_MASK 0x00020000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_SKIP_CC_CHECK_SC_GEN_PKT_SHIFT 17
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_SKIP_CC_CHECK_SC_GEN_PKT_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: ACCEPT_SC_GEN_STUFF_PKT [16:16] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_ACCEPT_SC_GEN_STUFF_PKT_MASK 0x00010000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_ACCEPT_SC_GEN_STUFF_PKT_SHIFT 16
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_ACCEPT_SC_GEN_STUFF_PKT_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: CRC_INIT_VALUE [15:08] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_CRC_INIT_VALUE_MASK 0x0000ff00
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_CRC_INIT_VALUE_SHIFT 8
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_CRC_INIT_VALUE_DEFAULT 0x000000ff

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: MPEG_SCRAM_FLAG_CTRL [07:07] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_MPEG_SCRAM_FLAG_CTRL_MASK 0x00000080
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_MPEG_SCRAM_FLAG_CTRL_SHIFT 7
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_MPEG_SCRAM_FLAG_CTRL_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: GEN_STUFF_PKT [06:06] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_GEN_STUFF_PKT_MASK 0x00000040
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_GEN_STUFF_PKT_SHIFT 6
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_GEN_STUFF_PKT_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: HEADER_CNC_EN [05:05] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_HEADER_CNC_EN_MASK 0x00000020
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_HEADER_CNC_EN_SHIFT 5
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_HEADER_CNC_EN_DEFAULT 0x00000001

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: PARSER_TEI_IGNORE_POST_TSIO [04:04] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_PARSER_TEI_IGNORE_POST_TSIO_MASK 0x00000010
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_PARSER_TEI_IGNORE_POST_TSIO_SHIFT 4
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_PARSER_TEI_IGNORE_POST_TSIO_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: TIMESTAMP_INSERT_EN [03:03] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_TIMESTAMP_INSERT_EN_MASK 0x00000008
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_TIMESTAMP_INSERT_EN_SHIFT 3
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_TIMESTAMP_INSERT_EN_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: TSIO_FUNC_ENABLE [02:02] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_TSIO_FUNC_ENABLE_MASK 0x00000004
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_TSIO_FUNC_ENABLE_SHIFT 2
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_TSIO_FUNC_ENABLE_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: CALIB_ENABLE [01:01] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_CALIB_ENABLE_MASK 0x00000002
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_CALIB_ENABLE_SHIFT 1
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_CALIB_ENABLE_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CTRL :: TSIO_ENABLE [00:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_TSIO_ENABLE_MASK  0x00000001
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_TSIO_ENABLE_SHIFT 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CTRL_TSIO_ENABLE_DEFAULT 0x00000000

/***************************************************************************
 *TSIO_CLK_SELECT - TSIO Serial Clock Select
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CLK_SELECT :: reserved0 [31:02] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CLK_SELECT_reserved0_MASK 0xfffffffc
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CLK_SELECT_reserved0_SHIFT 2

/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_CLK_SELECT :: CLK_SELECT [01:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CLK_SELECT_CLK_SELECT_MASK 0x00000003
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CLK_SELECT_CLK_SELECT_SHIFT 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_CLK_SELECT_CLK_SELECT_DEFAULT 0x00000002

/***************************************************************************
 *STUFF_PKT_SERVICE_ID - TSIO Stuffing Packet ServiceID
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: STUFF_PKT_SERVICE_ID :: reserved0 [31:06] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_STUFF_PKT_SERVICE_ID_reserved0_MASK 0xffffffc0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_STUFF_PKT_SERVICE_ID_reserved0_SHIFT 6

/* XPT_TSIO_CONFIG_REGISTERS :: STUFF_PKT_SERVICE_ID :: SERVICE_ID [05:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_STUFF_PKT_SERVICE_ID_SERVICE_ID_MASK 0x0000003f
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_STUFF_PKT_SERVICE_ID_SERVICE_ID_SHIFT 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_STUFF_PKT_SERVICE_ID_SERVICE_ID_DEFAULT 0x00000000

/***************************************************************************
 *TX_CNC_PING_BUF_CTRL - Transmit C&C Ping Buffer Ctrl
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PING_BUF_CTRL :: reserved0 [31:17] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL_reserved0_MASK 0xfffe0000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL_reserved0_SHIFT 17

/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PING_BUF_CTRL :: BUF_RDY [16:16] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL_BUF_RDY_MASK 0x00010000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL_BUF_RDY_SHIFT 16
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL_BUF_RDY_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PING_BUF_CTRL :: reserved1 [15:09] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL_reserved1_MASK 0x0000fe00
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL_reserved1_SHIFT 9

/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PING_BUF_CTRL :: BUF_SIZE [08:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL_BUF_SIZE_MASK 0x000001ff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL_BUF_SIZE_SHIFT 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUF_CTRL_BUF_SIZE_DEFAULT 0x00000000

/***************************************************************************
 *TX_CNC_PONG_BUF_CTRL - Transmit C&C Pong Buffer Ctrl
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PONG_BUF_CTRL :: reserved0 [31:17] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL_reserved0_MASK 0xfffe0000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL_reserved0_SHIFT 17

/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PONG_BUF_CTRL :: BUF_RDY [16:16] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL_BUF_RDY_MASK 0x00010000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL_BUF_RDY_SHIFT 16
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL_BUF_RDY_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PONG_BUF_CTRL :: reserved1 [15:09] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL_reserved1_MASK 0x0000fe00
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL_reserved1_SHIFT 9

/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PONG_BUF_CTRL :: BUF_SIZE [08:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL_BUF_SIZE_MASK 0x000001ff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL_BUF_SIZE_SHIFT 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUF_CTRL_BUF_SIZE_DEFAULT 0x00000000

/***************************************************************************
 *RX_CNC_PING_BUF_STATUS - Received C&C Ping Buffer Status
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PING_BUF_STATUS :: reserved0 [31:17] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS_reserved0_MASK 0xfffe0000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS_reserved0_SHIFT 17

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PING_BUF_STATUS :: BUF_RDY [16:16] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS_BUF_RDY_MASK 0x00010000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS_BUF_RDY_SHIFT 16
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS_BUF_RDY_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PING_BUF_STATUS :: reserved1 [15:09] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS_reserved1_MASK 0x0000fe00
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS_reserved1_SHIFT 9

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PING_BUF_STATUS :: BUF_SIZE [08:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS_BUF_SIZE_MASK 0x000001ff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS_BUF_SIZE_SHIFT 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUF_STATUS_BUF_SIZE_DEFAULT 0x00000000

/***************************************************************************
 *RX_CNC_PONG_BUF_STATUS - Received C&C Pong Buffer Status
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PONG_BUF_STATUS :: reserved0 [31:17] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS_reserved0_MASK 0xfffe0000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS_reserved0_SHIFT 17

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PONG_BUF_STATUS :: BUF_RDY [16:16] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS_BUF_RDY_MASK 0x00010000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS_BUF_RDY_SHIFT 16
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS_BUF_RDY_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PONG_BUF_STATUS :: reserved1 [15:09] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS_reserved1_MASK 0x0000fe00
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS_reserved1_SHIFT 9

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PONG_BUF_STATUS :: BUF_SIZE [08:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS_BUF_SIZE_MASK 0x000001ff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS_BUF_SIZE_SHIFT 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUF_STATUS_BUF_SIZE_DEFAULT 0x00000000

/***************************************************************************
 *RX_CNC_ERROR_STATUS - Received C&C Error Status
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_ERROR_STATUS :: reserved0 [31:06] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_reserved0_MASK 0xffffffc0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_reserved0_SHIFT 6

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_ERROR_STATUS :: PONG_CNC_LEN_ERR [05:05] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PONG_CNC_LEN_ERR_MASK 0x00000020
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PONG_CNC_LEN_ERR_SHIFT 5
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PONG_CNC_LEN_ERR_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_ERROR_STATUS :: PONG_CNC_OVERFLOW_ERR [04:04] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PONG_CNC_OVERFLOW_ERR_MASK 0x00000010
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PONG_CNC_OVERFLOW_ERR_SHIFT 4
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PONG_CNC_OVERFLOW_ERR_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_ERROR_STATUS :: PONG_CNC_ID_ERR [03:03] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PONG_CNC_ID_ERR_MASK 0x00000008
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PONG_CNC_ID_ERR_SHIFT 3
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PONG_CNC_ID_ERR_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_ERROR_STATUS :: PING_CNC_LEN_ERR [02:02] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PING_CNC_LEN_ERR_MASK 0x00000004
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PING_CNC_LEN_ERR_SHIFT 2
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PING_CNC_LEN_ERR_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_ERROR_STATUS :: PING_CNC_OVERFLOW_ERR [01:01] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PING_CNC_OVERFLOW_ERR_MASK 0x00000002
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PING_CNC_OVERFLOW_ERR_SHIFT 1
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PING_CNC_OVERFLOW_ERR_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_ERROR_STATUS :: PING_CNC_ID_ERR [00:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PING_CNC_ID_ERR_MASK 0x00000001
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PING_CNC_ID_ERR_SHIFT 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_ERROR_STATUS_PING_CNC_ID_ERR_DEFAULT 0x00000000

/***************************************************************************
 *CTS_CHECK_EN - Enable usage of the received CTS values during pipe arbitration
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: CTS_CHECK_EN :: reserved0 [31:01] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CTS_CHECK_EN_reserved0_MASK 0xfffffffe
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CTS_CHECK_EN_reserved0_SHIFT 1

/* XPT_TSIO_CONFIG_REGISTERS :: CTS_CHECK_EN :: CTS_CHKEN [00:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CTS_CHECK_EN_CTS_CHKEN_MASK 0x00000001
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CTS_CHECK_EN_CTS_CHKEN_SHIFT 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CTS_CHECK_EN_CTS_CHKEN_DEFAULT 0x00000001

/***************************************************************************
 *RX_CTS_STATUS - Received CTS status from the smartcard
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: RX_CTS_STATUS :: reserved0 [31:01] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CTS_STATUS_reserved0_MASK 0xfffffffe
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CTS_STATUS_reserved0_SHIFT 1

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CTS_STATUS :: RX_CTS [00:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CTS_STATUS_RX_CTS_MASK   0x00000001
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CTS_STATUS_RX_CTS_SHIFT  0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CTS_STATUS_RX_CTS_DEFAULT 0x00000001

/***************************************************************************
 *TX_PKT_DROP_STATUS - Tx Packet drop status per pipe
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TX_PKT_DROP_STATUS :: reserved0 [31:03] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_reserved0_MASK 0xfffffff8
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_reserved0_SHIFT 3

/* XPT_TSIO_CONFIG_REGISTERS :: TX_PKT_DROP_STATUS :: PKT_DROP_BEST_EFFORT_PIPE [02:02] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_PKT_DROP_BEST_EFFORT_PIPE_MASK 0x00000004
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_PKT_DROP_BEST_EFFORT_PIPE_SHIFT 2
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_PKT_DROP_BEST_EFFORT_PIPE_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TX_PKT_DROP_STATUS :: PKT_DROP_PACING_PIPE [01:01] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_PKT_DROP_PACING_PIPE_MASK 0x00000002
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_PKT_DROP_PACING_PIPE_SHIFT 1
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_PKT_DROP_PACING_PIPE_DEFAULT 0x00000000

/* XPT_TSIO_CONFIG_REGISTERS :: TX_PKT_DROP_STATUS :: PKT_DROP_LIVE_PIPE [00:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_PKT_DROP_LIVE_PIPE_MASK 0x00000001
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_PKT_DROP_LIVE_PIPE_SHIFT 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_PKT_DROP_STATUS_PKT_DROP_LIVE_PIPE_DEFAULT 0x00000000

/***************************************************************************
 *CLK_SEL_VALUE - TSIO Clk select final value as realised by h/w
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: CLK_SEL_VALUE :: reserved0 [31:02] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CLK_SEL_VALUE_reserved0_MASK 0xfffffffc
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CLK_SEL_VALUE_reserved0_SHIFT 2

/* XPT_TSIO_CONFIG_REGISTERS :: CLK_SEL_VALUE :: CLK_SEL [01:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CLK_SEL_VALUE_CLK_SEL_MASK  0x00000003
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_CLK_SEL_VALUE_CLK_SEL_SHIFT 0

/***************************************************************************
 *TV_STATUS - TV status register
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TV_STATUS :: reserved0 [31:08] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TV_STATUS_reserved0_MASK    0xffffff00
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TV_STATUS_reserved0_SHIFT   8

/* XPT_TSIO_CONFIG_REGISTERS :: TV_STATUS :: CRC_RESULT [07:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TV_STATUS_CRC_RESULT_MASK   0x000000ff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TV_STATUS_CRC_RESULT_SHIFT  0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TV_STATUS_CRC_RESULT_DEFAULT 0x00000000

/***************************************************************************
 *TSIO_DEBUG_0 - TSIO Debug Register
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_DEBUG_0 :: DEBUG_STATES [31:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_0_DEBUG_STATES_MASK 0xffffffff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_0_DEBUG_STATES_SHIFT 0

/***************************************************************************
 *TSIO_DEBUG_1 - TSIO Debug Register
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_DEBUG_1 :: DEBUG_STATES [31:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_1_DEBUG_STATES_MASK 0xffffffff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_1_DEBUG_STATES_SHIFT 0

/***************************************************************************
 *TSIO_DEBUG_2 - TSIO Debug Register
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_DEBUG_2 :: DEBUG_STATES [31:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_2_DEBUG_STATES_MASK 0xffffffff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_2_DEBUG_STATES_SHIFT 0

/***************************************************************************
 *TSIO_DEBUG_3 - TSIO Debug Register
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_DEBUG_3 :: DEBUG_STATES [31:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_3_DEBUG_STATES_MASK 0xffffffff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_3_DEBUG_STATES_SHIFT 0

/***************************************************************************
 *TSIO_DEBUG_4 - TSIO Debug Register
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_DEBUG_4 :: DEBUG_STATES [31:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_4_DEBUG_STATES_MASK 0xffffffff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_4_DEBUG_STATES_SHIFT 0

/***************************************************************************
 *TSIO_DEBUG_5 - TSIO Debug Register
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_DEBUG_5 :: DEBUG_STATES [31:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_5_DEBUG_STATES_MASK 0xffffffff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_5_DEBUG_STATES_SHIFT 0

/***************************************************************************
 *TSIO_DEBUG_6 - TSIO Debug Register
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_DEBUG_6 :: DEBUG_STATES [31:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_6_DEBUG_STATES_MASK 0xffffffff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_6_DEBUG_STATES_SHIFT 0

/***************************************************************************
 *TSIO_DEBUG_7 - TSIO Debug Register
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TSIO_DEBUG_7 :: DEBUG_STATES [31:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_7_DEBUG_STATES_MASK 0xffffffff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TSIO_DEBUG_7_DEBUG_STATES_SHIFT 0

/***************************************************************************
 *RX_CONFIG_TABLE%i - RX Config Table
 ***************************************************************************/
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_ARRAY_BASE 0x00a0d100
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_ARRAY_START 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_ARRAY_END  63
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *RX_CONFIG_TABLE%i - RX Config Table
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: RX_CONFIG_TABLEi :: reserved0 [31:16] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_reserved0_MASK 0xffff0000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_reserved0_SHIFT 16

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CONFIG_TABLEi :: ALL_PASS_PID_CH_NUM_POST_TSIO [15:04] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_ALL_PASS_PID_CH_NUM_POST_TSIO_MASK 0x0000fff0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_ALL_PASS_PID_CH_NUM_POST_TSIO_SHIFT 4

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CONFIG_TABLEi :: reserved1 [03:03] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_reserved1_MASK 0x00000008
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_reserved1_SHIFT 3

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CONFIG_TABLEi :: PARSER_ACCEPT_NULL_PKT_POST_TSIO [02:02] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_PARSER_ACCEPT_NULL_PKT_POST_TSIO_MASK 0x00000004
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_PARSER_ACCEPT_NULL_PKT_POST_TSIO_SHIFT 2

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CONFIG_TABLEi :: PARSER_ALL_PASS_CTRL_POST_TSIO [01:01] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_PARSER_ALL_PASS_CTRL_POST_TSIO_MASK 0x00000002
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_PARSER_ALL_PASS_CTRL_POST_TSIO_SHIFT 1

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CONFIG_TABLEi :: TSIO_PARSER_EN [00:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_TSIO_PARSER_EN_MASK 0x00000001
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CONFIG_TABLEi_TSIO_PARSER_EN_SHIFT 0


/***************************************************************************
 *TX_CNC_PING_BUFFER%i - Transmit C&C message data buffer
 ***************************************************************************/
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUFFERi_ARRAY_BASE 0x00a0d200
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUFFERi_ARRAY_START 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUFFERi_ARRAY_END 257
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUFFERi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *TX_CNC_PING_BUFFER%i - Transmit C&C message data buffer
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PING_BUFFERi :: reserved0 [31:08] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUFFERi_reserved0_MASK 0xffffff00
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUFFERi_reserved0_SHIFT 8

/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PING_BUFFERi :: CNC_DATA [07:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUFFERi_CNC_DATA_MASK 0x000000ff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PING_BUFFERi_CNC_DATA_SHIFT 0


/***************************************************************************
 *TX_CNC_PONG_BUFFER%i - Transmit C&C message data buffer
 ***************************************************************************/
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUFFERi_ARRAY_BASE 0x00a0d800
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUFFERi_ARRAY_START 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUFFERi_ARRAY_END 257
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUFFERi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *TX_CNC_PONG_BUFFER%i - Transmit C&C message data buffer
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PONG_BUFFERi :: reserved0 [31:08] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUFFERi_reserved0_MASK 0xffffff00
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUFFERi_reserved0_SHIFT 8

/* XPT_TSIO_CONFIG_REGISTERS :: TX_CNC_PONG_BUFFERi :: CNC_DATA [07:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUFFERi_CNC_DATA_MASK 0x000000ff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_TX_CNC_PONG_BUFFERi_CNC_DATA_SHIFT 0


/***************************************************************************
 *RX_CNC_PING_BUFFER%i - Receive C&C message data buffer
 ***************************************************************************/
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUFFERi_ARRAY_BASE 0x00a0e000
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUFFERi_ARRAY_START 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUFFERi_ARRAY_END 257
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUFFERi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *RX_CNC_PING_BUFFER%i - Receive C&C message data buffer
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PING_BUFFERi :: reserved0 [31:08] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUFFERi_reserved0_MASK 0xffffff00
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUFFERi_reserved0_SHIFT 8

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PING_BUFFERi :: CNC_DATA [07:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUFFERi_CNC_DATA_MASK 0x000000ff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PING_BUFFERi_CNC_DATA_SHIFT 0


/***************************************************************************
 *RX_CNC_PONG_BUFFER%i - Receive C&C message data buffer
 ***************************************************************************/
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUFFERi_ARRAY_BASE 0x00a0e600
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUFFERi_ARRAY_START 0
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUFFERi_ARRAY_END 257
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUFFERi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *RX_CNC_PONG_BUFFER%i - Receive C&C message data buffer
 ***************************************************************************/
/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PONG_BUFFERi :: reserved0 [31:08] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUFFERi_reserved0_MASK 0xffffff00
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUFFERi_reserved0_SHIFT 8

/* XPT_TSIO_CONFIG_REGISTERS :: RX_CNC_PONG_BUFFERi :: CNC_DATA [07:00] */
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUFFERi_CNC_DATA_MASK 0x000000ff
#define BCHP_XPT_TSIO_CONFIG_REGISTERS_RX_CNC_PONG_BUFFERi_CNC_DATA_SHIFT 0


#endif /* #ifndef BCHP_XPT_TSIO_CONFIG_REGISTERS_H__ */

/* End of File */
