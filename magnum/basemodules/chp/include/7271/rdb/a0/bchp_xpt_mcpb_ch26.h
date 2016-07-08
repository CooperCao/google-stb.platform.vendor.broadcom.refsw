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
 * Date:           Generated on               Fri Aug 21 14:43:27 2015
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

#ifndef BCHP_XPT_MCPB_CH26_H__
#define BCHP_XPT_MCPB_CH26_H__

/***************************************************************************
 *XPT_MCPB_CH26 - MCPB Channel 26 Configuration
 ***************************************************************************/
#define BCHP_XPT_MCPB_CH26_DMA_DESC_CONTROL      0x20a74000 /* [RW] MCPB Channel x Descriptor control information */
#define BCHP_XPT_MCPB_CH26_DMA_DATA_CONTROL      0x20a74004 /* [RW] MCPB Channel x Data control information */
#define BCHP_XPT_MCPB_CH26_DMA_CURR_DESC_ADDRESS 0x20a74008 /* [RW] MCPB Channel x Current Descriptor address information */
#define BCHP_XPT_MCPB_CH26_DMA_NEXT_DESC_ADDRESS 0x20a7400c /* [RW] MCPB Channel x Next Descriptor address information */
#define BCHP_XPT_MCPB_CH26_DMA_BUFF_BASE_ADDRESS_UPPER 0x20a74010 /* [RW] MCPB Channel x Data Buffer Base address */
#define BCHP_XPT_MCPB_CH26_DMA_BUFF_BASE_ADDRESS_LOWER 0x20a74014 /* [RW] MCPB Channel x Data Buffer Base address */
#define BCHP_XPT_MCPB_CH26_DMA_BUFF_END_ADDRESS_UPPER 0x20a74018 /* [RW] MCPB Channel x Data Buffer End address */
#define BCHP_XPT_MCPB_CH26_DMA_BUFF_END_ADDRESS_LOWER 0x20a7401c /* [RW] MCPB Channel x Data Buffer End address */
#define BCHP_XPT_MCPB_CH26_DMA_BUFF_CURR_RD_ADDRESS_UPPER 0x20a74020 /* [RW] MCPB Channel x Current Data Buffer Read address */
#define BCHP_XPT_MCPB_CH26_DMA_BUFF_CURR_RD_ADDRESS_LOWER 0x20a74024 /* [RW] MCPB Channel x Current Data Buffer Read address */
#define BCHP_XPT_MCPB_CH26_DMA_BUFF_WRITE_ADDRESS_UPPER 0x20a74028 /* [RW] MCPB Channel x Data Buffer Write address */
#define BCHP_XPT_MCPB_CH26_DMA_BUFF_WRITE_ADDRESS_LOWER 0x20a7402c /* [RW] MCPB Channel x Data Buffer Write address */
#define BCHP_XPT_MCPB_CH26_DMA_STATUS_0          0x20a74030 /* [RW] MCPB Channel x Status information */
#define BCHP_XPT_MCPB_CH26_DMA_STATUS_1          0x20a74034 /* [RW] MCPB Channel x CRC value */
#define BCHP_XPT_MCPB_CH26_DMA_STATUS_2          0x20a74038 /* [RW] MCPB Channel x Manual mode status */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT0_STATUS_0 0x20a7403c /* [RW] MCPB channel x Descriptor Slot 0 status information */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT0_STATUS_1 0x20a74040 /* [RW] MCPB channel x Descriptor Slot 0 status information */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT0_CURR_DESC_ADDR 0x20a74044 /* [RW] MCPB Channel x  Descriptor Slot 0 Current Descriptor Address */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT0_CURR_DATA_ADDR_UPPER 0x20a74048 /* [RW] MCPB Channel x  Descriptor Slot 0 Current Data Address */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT0_CURR_DATA_ADDR_LOWER 0x20a7404c /* [RW] MCPB Channel x  Descriptor Slot 0 Current Data Address */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT0_NEXT_TIMESTAMP 0x20a74050 /* [RW] MCPB Channel x Descriptor Slot 0 Next Packet Timestamp */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT0_PKT2PKT_TIMESTAMP_DELTA 0x20a74054 /* [RW] MCPB Channel x Descriptor Slot 0 Packet to packet Timestamp delta */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT1_STATUS_0 0x20a74058 /* [RW] MCPB channel x Descriptor Slot 1 status information */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT1_STATUS_1 0x20a7405c /* [RW] MCPB channel x Descriptor Slot 1 status information */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT1_CURR_DESC_ADDR 0x20a74060 /* [RW] MCPB Channel x  Descriptor Slot 1 Current Descriptor Address */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT1_CURR_DATA_ADDR_UPPER 0x20a74064 /* [RW] MCPB Channel x  Descriptor Slot 1 Current Data Address */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT1_CURR_DATA_ADDR_LOWER 0x20a74068 /* [RW] MCPB Channel x  Descriptor Slot 1 Current Data Address */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT1_NEXT_TIMESTAMP 0x20a7406c /* [RW] MCPB Channel x Descriptor Slot 1 Next Packet Timestamp */
#define BCHP_XPT_MCPB_CH26_DMA_DESC_SLOT1_PKT2PKT_TIMESTAMP_DELTA 0x20a74070 /* [RW] MCPB Channel x Descriptor Slot 1 Packet to packet Timestamp delta */
#define BCHP_XPT_MCPB_CH26_SP_PKT_LEN            0x20a74074 /* [RW] MCPB Channel x Packet length control */
#define BCHP_XPT_MCPB_CH26_SP_PARSER_CTRL        0x20a74078 /* [RW] MCPB Channel x Parser control */
#define BCHP_XPT_MCPB_CH26_SP_PARSER_CTRL1       0x20a7407c /* [RW] MCPB Channel x Parser control 1 */
#define BCHP_XPT_MCPB_CH26_SP_TS_CONFIG          0x20a74080 /* [RW] MCPB Channel x TS Configuration */
#define BCHP_XPT_MCPB_CH26_SP_PES_ES_CONFIG      0x20a74084 /* [RW] MCPB Channel x PES and ES Configuration */
#define BCHP_XPT_MCPB_CH26_SP_PES_SYNC_COUNTER   0x20a74088 /* [RW] MCPB Channel x PES Sync counter */
#define BCHP_XPT_MCPB_CH26_SP_ASF_CONFIG         0x20a7408c /* [RW] MCPB Channel x ASF Configuration */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_0        0x20a74090 /* [RW] MCPB Channel x Stream Processor State Register 0 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_1        0x20a74094 /* [RW] MCPB Channel x Stream Processor State Register 1 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_2        0x20a74098 /* [RW] MCPB Channel x Stream Processor State Register 2 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_3        0x20a7409c /* [RW] MCPB Channel x Stream Processor State Register 3 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_4        0x20a740a0 /* [RW] MCPB Channel x Stream Processor State Register 4 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_5        0x20a740a4 /* [RW] MCPB Channel x Stream Processor State Register 5 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_6        0x20a740a8 /* [RW] MCPB Channel x Stream Processor State Register 6 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_7        0x20a740ac /* [RW] MCPB Channel x Stream Processor State Register 7 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_8        0x20a740b0 /* [RW] MCPB Channel x Stream Processor State Register 8 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_9        0x20a740b4 /* [RW] MCPB Channel x Stream Processor State Register 9 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_10       0x20a740b8 /* [RW] MCPB Channel x Stream Processor State Register 10 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_11       0x20a740bc /* [RW] MCPB Channel x Stream Processor State Register 11 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_12       0x20a740c0 /* [RW] MCPB Channel x Stream Processor State Register 12 */
#define BCHP_XPT_MCPB_CH26_SP_STATE_REG_13       0x20a740c4 /* [RW] MCPB Channel x Stream Processor State Register 13 */
#define BCHP_XPT_MCPB_CH26_DMA_BBUFF_CTRL        0x20a740c8 /* [RW] MCPB Channel x Burst buffer control */
#define BCHP_XPT_MCPB_CH26_DMA_BBUFF_CRC         0x20a740cc /* [RW] MCPB Channel x Current CRC value */
#define BCHP_XPT_MCPB_CH26_DMA_BBUFF0_RW_STATUS  0x20a740d0 /* [RW] MCPB Channel x Burst buffer 0 data specific information */
#define BCHP_XPT_MCPB_CH26_DMA_BBUFF0_RO_STATUS  0x20a740d4 /* [RW] MCPB Channel x Burst buffer 0 control specific information */
#define BCHP_XPT_MCPB_CH26_DMA_BBUFF1_RW_STATUS  0x20a740d8 /* [RW] MCPB Channel x Burst buffer 1 data specific information */
#define BCHP_XPT_MCPB_CH26_DMA_BBUFF1_RO_STATUS  0x20a740dc /* [RW] MCPB Channel x Burst buffer 1 control specific information */
#define BCHP_XPT_MCPB_CH26_TMEU_BLOCKOUT_CTRL    0x20a740e0 /* [RW] MCPB Channel x Blockout control information */
#define BCHP_XPT_MCPB_CH26_TMEU_NEXT_BO_MON      0x20a740e4 /* [RW] MCPB Channel x next Blockout monitor information */
#define BCHP_XPT_MCPB_CH26_TMEU_TIMING_CTRL      0x20a740e8 /* [RW] MCPB Channel x next Blockout monitor information */
#define BCHP_XPT_MCPB_CH26_TMEU_REF_DIFF_VALUE_TS_MBOX 0x20a740ec /* [RW] MCPB Channel x reference difference value and next Timestamp information */
#define BCHP_XPT_MCPB_CH26_TMEU_TS_ERR_BOUND_EARLY 0x20a740f0 /* [RW] MCPB Channel x TS error bound early information */
#define BCHP_XPT_MCPB_CH26_TMEU_TS_ERR_BOUND_LATE 0x20a740f4 /* [RW] MCPB Channel x TS error bound late information */
#define BCHP_XPT_MCPB_CH26_TMEU_NEXT_GPC_MON     0x20a740f8 /* [RW] MCPB Channel x next Global Pacing Counter and Timestamp monitor information */
#define BCHP_XPT_MCPB_CH26_TMEU_REF_DIFF_VALUE_SIGN 0x20a740fc /* [RW] MCPB Channel x reference difference value sign information */
#define BCHP_XPT_MCPB_CH26_TMEU_PES_PACING_CTRL  0x20a74100 /* [RW] MCPB Channel x PES pacing control information */
#define BCHP_XPT_MCPB_CH26_TMEU_SLOT_STATUS      0x20a74104 /* [RW] MCPB Channel x Slot 0 and Slot 1 information */
#define BCHP_XPT_MCPB_CH26_TMEU_TIMING_INFO_SLOT0_REG1 0x20a74108 /* [RW] MCPB Channel x timing information for Slot 0 */
#define BCHP_XPT_MCPB_CH26_TMEU_TIMING_INFO_SLOT0_REG2 0x20a7410c /* [RW] MCPB Channel x timing information for Slot 0 */
#define BCHP_XPT_MCPB_CH26_TMEU_TIMING_INFO_SLOT1_REG1 0x20a74110 /* [RW] MCPB Channel x timing information for Slot 1 */
#define BCHP_XPT_MCPB_CH26_TMEU_TIMING_INFO_SLOT1_REG2 0x20a74114 /* [RW] MCPB Channel x timing information for Slot 1 */
#define BCHP_XPT_MCPB_CH26_TMEU_TIMING_INFO_LAST_TIMESTAMP_DELTA 0x20a74118 /* [RW] MCPB Channel x last TS delta value */
#define BCHP_XPT_MCPB_CH26_TMEU_TIMING_INFO_LAST_NEXT_TIMESTAMP 0x20a7411c /* [RW] MCPB Channel x last NEXT TS value */
#define BCHP_XPT_MCPB_CH26_DCPM_STATUS           0x20a74120 /* [RW] MCPB Channel x DCPM status information */
#define BCHP_XPT_MCPB_CH26_DCPM_DESC_ADDR        0x20a74124 /* [RW] MCPB Channel x DCPM descriptor address information */
#define BCHP_XPT_MCPB_CH26_DCPM_DESC_DONE_INT_ADDR 0x20a74128 /* [RW] MCPB Channel x DCPM descriptor done interrupt address information */
#define BCHP_XPT_MCPB_CH26_DCPM_PAUSE_AFTER_GROUP_PACKETS_CTRL 0x20a7412c /* [RW] MCPB Channel x Pause after group of packets control information */
#define BCHP_XPT_MCPB_CH26_DCPM_PAUSE_AFTER_GROUP_PACKETS_PKT_COUNTER 0x20a74130 /* [RW] MCPB Channel x Pause after group of packets local packet counter */
#define BCHP_XPT_MCPB_CH26_DCPM_LOCAL_PACKET_COUNTER 0x20a74134 /* [RW] MCPB Channel x local packet counter */
#define BCHP_XPT_MCPB_CH26_DCPM_DATA_ADDR_UPPER  0x20a74138 /* [RW] MCPB Channel x DCPM data address information */
#define BCHP_XPT_MCPB_CH26_DCPM_DATA_ADDR_LOWER  0x20a7413c /* [RW] MCPB Channel x DCPM data address information */
#define BCHP_XPT_MCPB_CH26_DCPM_CURR_DESC_ADDR   0x20a74140 /* [RW] MCPB Channel x DCPM current descriptor address information */
#define BCHP_XPT_MCPB_CH26_DCPM_SLOT_STATUS      0x20a74144 /* [RW] MCPB Channel x DCPM slot status information */
#define BCHP_XPT_MCPB_CH26_DCPM_DESC_ADDR_SLOT_0 0x20a74148 /* [RW] MCPB Channel x DCPM completed slot 0 descriptor address information */
#define BCHP_XPT_MCPB_CH26_DCPM_DATA_ADDR_SLOT_0_UPPER 0x20a7414c /* [RW] MCPB Channel x DCPM completed slot 0 data address information */
#define BCHP_XPT_MCPB_CH26_DCPM_DATA_ADDR_SLOT_0_LOWER 0x20a74150 /* [RW] MCPB Channel x DCPM completed slot 0 data address information */
#define BCHP_XPT_MCPB_CH26_DCPM_DESC_ADDR_SLOT_1 0x20a74154 /* [RW] MCPB Channel x DCPM completed slot 1 descriptor address information */
#define BCHP_XPT_MCPB_CH26_DCPM_DATA_ADDR_SLOT_1_UPPER 0x20a74158 /* [RW] MCPB Channel x DCPM completed slot 1 data address information */
#define BCHP_XPT_MCPB_CH26_DCPM_DATA_ADDR_SLOT_1_LOWER 0x20a7415c /* [RW] MCPB Channel x DCPM completed slot 1 data address information */

#endif /* #ifndef BCHP_XPT_MCPB_CH26_H__ */

/* End of File */
