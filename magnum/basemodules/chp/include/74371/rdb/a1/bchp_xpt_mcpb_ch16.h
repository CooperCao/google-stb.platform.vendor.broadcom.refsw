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
 * Date:           Generated on               Fri Feb 20 00:05:23 2015
 *                 Full Compile MD5 Checksum  f4a546a20d0bd1f244e0d6a139e85ce0
 *                     (minus title and desc)
 *                 MD5 Checksum               a9d9eeea3a1c30a122d08de69d07786c
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15715
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_XPT_MCPB_CH16_H__
#define BCHP_XPT_MCPB_CH16_H__

/***************************************************************************
 *XPT_MCPB_CH16 - MCPB Channel 16 Configuration
 ***************************************************************************/
#define BCHP_XPT_MCPB_CH16_DMA_DESC_CONTROL      0x00a72c00 /* [RW] MCPB Channel x Descriptor control information */
#define BCHP_XPT_MCPB_CH16_DMA_DATA_CONTROL      0x00a72c04 /* [RW] MCPB Channel x Data control information */
#define BCHP_XPT_MCPB_CH16_DMA_CURR_DESC_ADDRESS 0x00a72c08 /* [RW] MCPB Channel x Current Descriptor address information */
#define BCHP_XPT_MCPB_CH16_DMA_NEXT_DESC_ADDRESS 0x00a72c0c /* [RW] MCPB Channel x Next Descriptor address information */
#define BCHP_XPT_MCPB_CH16_DMA_BUFF_BASE_ADDRESS_UPPER 0x00a72c10 /* [RW] MCPB Channel x Data Buffer Base address */
#define BCHP_XPT_MCPB_CH16_DMA_BUFF_BASE_ADDRESS_LOWER 0x00a72c14 /* [RW] MCPB Channel x Data Buffer Base address */
#define BCHP_XPT_MCPB_CH16_DMA_BUFF_END_ADDRESS_UPPER 0x00a72c18 /* [RW] MCPB Channel x Data Buffer End address */
#define BCHP_XPT_MCPB_CH16_DMA_BUFF_END_ADDRESS_LOWER 0x00a72c1c /* [RW] MCPB Channel x Data Buffer End address */
#define BCHP_XPT_MCPB_CH16_DMA_BUFF_CURR_RD_ADDRESS_UPPER 0x00a72c20 /* [RW] MCPB Channel x Current Data Buffer Read address */
#define BCHP_XPT_MCPB_CH16_DMA_BUFF_CURR_RD_ADDRESS_LOWER 0x00a72c24 /* [RW] MCPB Channel x Current Data Buffer Read address */
#define BCHP_XPT_MCPB_CH16_DMA_BUFF_WRITE_ADDRESS_UPPER 0x00a72c28 /* [RW] MCPB Channel x Data Buffer Write address */
#define BCHP_XPT_MCPB_CH16_DMA_BUFF_WRITE_ADDRESS_LOWER 0x00a72c2c /* [RW] MCPB Channel x Data Buffer Write address */
#define BCHP_XPT_MCPB_CH16_DMA_STATUS_0          0x00a72c30 /* [RW] MCPB Channel x Status information */
#define BCHP_XPT_MCPB_CH16_DMA_STATUS_1          0x00a72c34 /* [RW] MCPB Channel x CRC value */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT0_STATUS_0 0x00a72c38 /* [RW] MCPB channel x Descriptor Slot 0 status information */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT0_STATUS_1 0x00a72c3c /* [RW] MCPB channel x Descriptor Slot 0 status information */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT0_CURR_DESC_ADDR 0x00a72c40 /* [RW] MCPB Channel x  Descriptor Slot 0 Current Descriptor Address */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT0_CURR_DATA_ADDR_UPPER 0x00a72c44 /* [RW] MCPB Channel x  Descriptor Slot 0 Current Data Address */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT0_CURR_DATA_ADDR_LOWER 0x00a72c48 /* [RW] MCPB Channel x  Descriptor Slot 0 Current Data Address */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT0_NEXT_TIMESTAMP 0x00a72c4c /* [RW] MCPB Channel x Descriptor Slot 0 Next Packet Timestamp */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT0_PKT2PKT_TIMESTAMP_DELTA 0x00a72c50 /* [RW] MCPB Channel x Descriptor Slot 0 Packet to packet Timestamp delta */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT1_STATUS_0 0x00a72c54 /* [RW] MCPB channel x Descriptor Slot 1 status information */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT1_STATUS_1 0x00a72c58 /* [RW] MCPB channel x Descriptor Slot 1 status information */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT1_CURR_DESC_ADDR 0x00a72c5c /* [RW] MCPB Channel x  Descriptor Slot 1 Current Descriptor Address */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT1_CURR_DATA_ADDR_UPPER 0x00a72c60 /* [RW] MCPB Channel x  Descriptor Slot 1 Current Data Address */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT1_CURR_DATA_ADDR_LOWER 0x00a72c64 /* [RW] MCPB Channel x  Descriptor Slot 1 Current Data Address */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT1_NEXT_TIMESTAMP 0x00a72c68 /* [RW] MCPB Channel x Descriptor Slot 1 Next Packet Timestamp */
#define BCHP_XPT_MCPB_CH16_DMA_DESC_SLOT1_PKT2PKT_TIMESTAMP_DELTA 0x00a72c6c /* [RW] MCPB Channel x Descriptor Slot 1 Packet to packet Timestamp delta */
#define BCHP_XPT_MCPB_CH16_SP_PKT_LEN            0x00a72c70 /* [RW] MCPB Channel x Packet length control */
#define BCHP_XPT_MCPB_CH16_SP_PARSER_CTRL        0x00a72c74 /* [RW] MCPB Channel x Parser control */
#define BCHP_XPT_MCPB_CH16_SP_TS_CONFIG          0x00a72c78 /* [RW] MCPB Channel x TS Configuration */
#define BCHP_XPT_MCPB_CH16_SP_PES_ES_CONFIG      0x00a72c7c /* [RW] MCPB Channel x PES and ES Configuration */
#define BCHP_XPT_MCPB_CH16_SP_PES_SYNC_COUNTER   0x00a72c80 /* [RW] MCPB Channel x PES Sync counter */
#define BCHP_XPT_MCPB_CH16_SP_ASF_CONFIG         0x00a72c84 /* [RW] MCPB Channel x ASF Configuration */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_0        0x00a72c88 /* [RW] MCPB Channel x Stream Processor State Register 0 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_1        0x00a72c8c /* [RW] MCPB Channel x Stream Processor State Register 1 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_2        0x00a72c90 /* [RW] MCPB Channel x Stream Processor State Register 2 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_3        0x00a72c94 /* [RW] MCPB Channel x Stream Processor State Register 3 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_4        0x00a72c98 /* [RW] MCPB Channel x Stream Processor State Register 4 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_5        0x00a72c9c /* [RW] MCPB Channel x Stream Processor State Register 5 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_6        0x00a72ca0 /* [RW] MCPB Channel x Stream Processor State Register 6 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_7        0x00a72ca4 /* [RW] MCPB Channel x Stream Processor State Register 7 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_8        0x00a72ca8 /* [RW] MCPB Channel x Stream Processor State Register 8 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_9        0x00a72cac /* [RW] MCPB Channel x Stream Processor State Register 9 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_10       0x00a72cb0 /* [RW] MCPB Channel x Stream Processor State Register 10 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_11       0x00a72cb4 /* [RW] MCPB Channel x Stream Processor State Register 11 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_12       0x00a72cb8 /* [RW] MCPB Channel x Stream Processor State Register 12 */
#define BCHP_XPT_MCPB_CH16_SP_STATE_REG_13       0x00a72cbc /* [RW] MCPB Channel x Stream Processor State Register 13 */
#define BCHP_XPT_MCPB_CH16_DMA_BBUFF_CTRL        0x00a72cc0 /* [RW] MCPB Channel x Burst buffer control */
#define BCHP_XPT_MCPB_CH16_DMA_BBUFF_CRC         0x00a72cc4 /* [RW] MCPB Channel x Current CRC value */
#define BCHP_XPT_MCPB_CH16_DMA_BBUFF0_RW_STATUS  0x00a72cc8 /* [RW] MCPB Channel x Burst buffer 0 data specific information */
#define BCHP_XPT_MCPB_CH16_DMA_BBUFF0_RO_STATUS  0x00a72ccc /* [RW] MCPB Channel x Burst buffer 0 control specific information */
#define BCHP_XPT_MCPB_CH16_DMA_BBUFF1_RW_STATUS  0x00a72cd0 /* [RW] MCPB Channel x Burst buffer 1 data specific information */
#define BCHP_XPT_MCPB_CH16_DMA_BBUFF1_RO_STATUS  0x00a72cd4 /* [RW] MCPB Channel x Burst buffer 1 control specific information */
#define BCHP_XPT_MCPB_CH16_TMEU_BLOCKOUT_CTRL    0x00a72cd8 /* [RW] MCPB Channel x Blockout control information */
#define BCHP_XPT_MCPB_CH16_TMEU_NEXT_BO_MON      0x00a72cdc /* [RW] MCPB Channel x next Blockout monitor information */
#define BCHP_XPT_MCPB_CH16_TMEU_TIMING_CTRL      0x00a72ce0 /* [RW] MCPB Channel x next Blockout monitor information */
#define BCHP_XPT_MCPB_CH16_TMEU_REF_DIFF_VALUE_TS_MBOX 0x00a72ce4 /* [RW] MCPB Channel x reference difference value and next Timestamp information */
#define BCHP_XPT_MCPB_CH16_TMEU_TS_ERR_BOUND_EARLY 0x00a72ce8 /* [RW] MCPB Channel x TS error bound early information */
#define BCHP_XPT_MCPB_CH16_TMEU_TS_ERR_BOUND_LATE 0x00a72cec /* [RW] MCPB Channel x TS error bound late information */
#define BCHP_XPT_MCPB_CH16_TMEU_NEXT_GPC_MON     0x00a72cf0 /* [RW] MCPB Channel x next Global Pacing Counter and Timestamp monitor information */
#define BCHP_XPT_MCPB_CH16_TMEU_REF_DIFF_VALUE_SIGN 0x00a72cf4 /* [RW] MCPB Channel x reference difference value sign information */
#define BCHP_XPT_MCPB_CH16_TMEU_PES_PACING_CTRL  0x00a72cf8 /* [RW] MCPB Channel x PES pacing control information */
#define BCHP_XPT_MCPB_CH16_TMEU_SLOT_STATUS      0x00a72cfc /* [RW] MCPB Channel x Slot 0 and Slot 1 information */
#define BCHP_XPT_MCPB_CH16_TMEU_TIMING_INFO_SLOT0_REG1 0x00a72d00 /* [RW] MCPB Channel x timing information for Slot 0 */
#define BCHP_XPT_MCPB_CH16_TMEU_TIMING_INFO_SLOT0_REG2 0x00a72d04 /* [RW] MCPB Channel x timing information for Slot 0 */
#define BCHP_XPT_MCPB_CH16_TMEU_TIMING_INFO_SLOT1_REG1 0x00a72d08 /* [RW] MCPB Channel x timing information for Slot 1 */
#define BCHP_XPT_MCPB_CH16_TMEU_TIMING_INFO_SLOT1_REG2 0x00a72d0c /* [RW] MCPB Channel x timing information for Slot 1 */
#define BCHP_XPT_MCPB_CH16_TMEU_TIMING_INFO_LAST_TIMESTAMP_DELTA 0x00a72d10 /* [RW] MCPB Channel x last TS delta value */
#define BCHP_XPT_MCPB_CH16_TMEU_TIMING_INFO_LAST_NEXT_TIMESTAMP 0x00a72d14 /* [RW] MCPB Channel x last NEXT TS value */
#define BCHP_XPT_MCPB_CH16_DCPM_STATUS           0x00a72d18 /* [RW] MCPB Channel x DCPM status information */
#define BCHP_XPT_MCPB_CH16_DCPM_DESC_ADDR        0x00a72d1c /* [RW] MCPB Channel x DCPM descriptor address information */
#define BCHP_XPT_MCPB_CH16_DCPM_DESC_DONE_INT_ADDR 0x00a72d20 /* [RW] MCPB Channel x DCPM descriptor done interrupt address information */
#define BCHP_XPT_MCPB_CH16_DCPM_PAUSE_AFTER_GROUP_PACKETS_CTRL 0x00a72d24 /* [RW] MCPB Channel x Pause after group of packets control information */
#define BCHP_XPT_MCPB_CH16_DCPM_PAUSE_AFTER_GROUP_PACKETS_PKT_COUNTER 0x00a72d28 /* [RW] MCPB Channel x Pause after group of packets local packet counter */
#define BCHP_XPT_MCPB_CH16_DCPM_LOCAL_PACKET_COUNTER 0x00a72d2c /* [RW] MCPB Channel x local packet counter */
#define BCHP_XPT_MCPB_CH16_DCPM_DATA_ADDR_UPPER  0x00a72d30 /* [RW] MCPB Channel x DCPM data address information */
#define BCHP_XPT_MCPB_CH16_DCPM_DATA_ADDR_LOWER  0x00a72d34 /* [RW] MCPB Channel x DCPM data address information */
#define BCHP_XPT_MCPB_CH16_DCPM_CURR_DESC_ADDR   0x00a72d38 /* [RW] MCPB Channel x DCPM current descriptor address information */
#define BCHP_XPT_MCPB_CH16_DCPM_SLOT_STATUS      0x00a72d3c /* [RW] MCPB Channel x DCPM slot status information */
#define BCHP_XPT_MCPB_CH16_DCPM_DESC_ADDR_SLOT_0 0x00a72d40 /* [RW] MCPB Channel x DCPM completed slot 0 descriptor address information */
#define BCHP_XPT_MCPB_CH16_DCPM_DATA_ADDR_SLOT_0_UPPER 0x00a72d44 /* [RW] MCPB Channel x DCPM completed slot 0 data address information */
#define BCHP_XPT_MCPB_CH16_DCPM_DATA_ADDR_SLOT_0_LOWER 0x00a72d48 /* [RW] MCPB Channel x DCPM completed slot 0 data address information */
#define BCHP_XPT_MCPB_CH16_DCPM_DESC_ADDR_SLOT_1 0x00a72d4c /* [RW] MCPB Channel x DCPM completed slot 1 descriptor address information */
#define BCHP_XPT_MCPB_CH16_DCPM_DATA_ADDR_SLOT_1_UPPER 0x00a72d50 /* [RW] MCPB Channel x DCPM completed slot 1 data address information */
#define BCHP_XPT_MCPB_CH16_DCPM_DATA_ADDR_SLOT_1_LOWER 0x00a72d54 /* [RW] MCPB Channel x DCPM completed slot 1 data address information */

#endif /* #ifndef BCHP_XPT_MCPB_CH16_H__ */

/* End of File */
