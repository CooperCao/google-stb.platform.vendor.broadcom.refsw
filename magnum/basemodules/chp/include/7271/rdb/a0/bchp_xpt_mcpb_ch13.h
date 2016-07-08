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
 * Date:           Generated on               Fri Aug 21 14:43:24 2015
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

#ifndef BCHP_XPT_MCPB_CH13_H__
#define BCHP_XPT_MCPB_CH13_H__

/***************************************************************************
 *XPT_MCPB_CH13 - MCPB Channel 13 Configuration
 ***************************************************************************/
#define BCHP_XPT_MCPB_CH13_DMA_DESC_CONTROL      0x20a72600 /* [RW] MCPB Channel x Descriptor control information */
#define BCHP_XPT_MCPB_CH13_DMA_DATA_CONTROL      0x20a72604 /* [RW] MCPB Channel x Data control information */
#define BCHP_XPT_MCPB_CH13_DMA_CURR_DESC_ADDRESS 0x20a72608 /* [RW] MCPB Channel x Current Descriptor address information */
#define BCHP_XPT_MCPB_CH13_DMA_NEXT_DESC_ADDRESS 0x20a7260c /* [RW] MCPB Channel x Next Descriptor address information */
#define BCHP_XPT_MCPB_CH13_DMA_BUFF_BASE_ADDRESS_UPPER 0x20a72610 /* [RW] MCPB Channel x Data Buffer Base address */
#define BCHP_XPT_MCPB_CH13_DMA_BUFF_BASE_ADDRESS_LOWER 0x20a72614 /* [RW] MCPB Channel x Data Buffer Base address */
#define BCHP_XPT_MCPB_CH13_DMA_BUFF_END_ADDRESS_UPPER 0x20a72618 /* [RW] MCPB Channel x Data Buffer End address */
#define BCHP_XPT_MCPB_CH13_DMA_BUFF_END_ADDRESS_LOWER 0x20a7261c /* [RW] MCPB Channel x Data Buffer End address */
#define BCHP_XPT_MCPB_CH13_DMA_BUFF_CURR_RD_ADDRESS_UPPER 0x20a72620 /* [RW] MCPB Channel x Current Data Buffer Read address */
#define BCHP_XPT_MCPB_CH13_DMA_BUFF_CURR_RD_ADDRESS_LOWER 0x20a72624 /* [RW] MCPB Channel x Current Data Buffer Read address */
#define BCHP_XPT_MCPB_CH13_DMA_BUFF_WRITE_ADDRESS_UPPER 0x20a72628 /* [RW] MCPB Channel x Data Buffer Write address */
#define BCHP_XPT_MCPB_CH13_DMA_BUFF_WRITE_ADDRESS_LOWER 0x20a7262c /* [RW] MCPB Channel x Data Buffer Write address */
#define BCHP_XPT_MCPB_CH13_DMA_STATUS_0          0x20a72630 /* [RW] MCPB Channel x Status information */
#define BCHP_XPT_MCPB_CH13_DMA_STATUS_1          0x20a72634 /* [RW] MCPB Channel x CRC value */
#define BCHP_XPT_MCPB_CH13_DMA_STATUS_2          0x20a72638 /* [RW] MCPB Channel x Manual mode status */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT0_STATUS_0 0x20a7263c /* [RW] MCPB channel x Descriptor Slot 0 status information */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT0_STATUS_1 0x20a72640 /* [RW] MCPB channel x Descriptor Slot 0 status information */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT0_CURR_DESC_ADDR 0x20a72644 /* [RW] MCPB Channel x  Descriptor Slot 0 Current Descriptor Address */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT0_CURR_DATA_ADDR_UPPER 0x20a72648 /* [RW] MCPB Channel x  Descriptor Slot 0 Current Data Address */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT0_CURR_DATA_ADDR_LOWER 0x20a7264c /* [RW] MCPB Channel x  Descriptor Slot 0 Current Data Address */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT0_NEXT_TIMESTAMP 0x20a72650 /* [RW] MCPB Channel x Descriptor Slot 0 Next Packet Timestamp */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT0_PKT2PKT_TIMESTAMP_DELTA 0x20a72654 /* [RW] MCPB Channel x Descriptor Slot 0 Packet to packet Timestamp delta */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT1_STATUS_0 0x20a72658 /* [RW] MCPB channel x Descriptor Slot 1 status information */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT1_STATUS_1 0x20a7265c /* [RW] MCPB channel x Descriptor Slot 1 status information */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT1_CURR_DESC_ADDR 0x20a72660 /* [RW] MCPB Channel x  Descriptor Slot 1 Current Descriptor Address */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT1_CURR_DATA_ADDR_UPPER 0x20a72664 /* [RW] MCPB Channel x  Descriptor Slot 1 Current Data Address */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT1_CURR_DATA_ADDR_LOWER 0x20a72668 /* [RW] MCPB Channel x  Descriptor Slot 1 Current Data Address */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT1_NEXT_TIMESTAMP 0x20a7266c /* [RW] MCPB Channel x Descriptor Slot 1 Next Packet Timestamp */
#define BCHP_XPT_MCPB_CH13_DMA_DESC_SLOT1_PKT2PKT_TIMESTAMP_DELTA 0x20a72670 /* [RW] MCPB Channel x Descriptor Slot 1 Packet to packet Timestamp delta */
#define BCHP_XPT_MCPB_CH13_SP_PKT_LEN            0x20a72674 /* [RW] MCPB Channel x Packet length control */
#define BCHP_XPT_MCPB_CH13_SP_PARSER_CTRL        0x20a72678 /* [RW] MCPB Channel x Parser control */
#define BCHP_XPT_MCPB_CH13_SP_PARSER_CTRL1       0x20a7267c /* [RW] MCPB Channel x Parser control 1 */
#define BCHP_XPT_MCPB_CH13_SP_TS_CONFIG          0x20a72680 /* [RW] MCPB Channel x TS Configuration */
#define BCHP_XPT_MCPB_CH13_SP_PES_ES_CONFIG      0x20a72684 /* [RW] MCPB Channel x PES and ES Configuration */
#define BCHP_XPT_MCPB_CH13_SP_PES_SYNC_COUNTER   0x20a72688 /* [RW] MCPB Channel x PES Sync counter */
#define BCHP_XPT_MCPB_CH13_SP_ASF_CONFIG         0x20a7268c /* [RW] MCPB Channel x ASF Configuration */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_0        0x20a72690 /* [RW] MCPB Channel x Stream Processor State Register 0 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_1        0x20a72694 /* [RW] MCPB Channel x Stream Processor State Register 1 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_2        0x20a72698 /* [RW] MCPB Channel x Stream Processor State Register 2 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_3        0x20a7269c /* [RW] MCPB Channel x Stream Processor State Register 3 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_4        0x20a726a0 /* [RW] MCPB Channel x Stream Processor State Register 4 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_5        0x20a726a4 /* [RW] MCPB Channel x Stream Processor State Register 5 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_6        0x20a726a8 /* [RW] MCPB Channel x Stream Processor State Register 6 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_7        0x20a726ac /* [RW] MCPB Channel x Stream Processor State Register 7 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_8        0x20a726b0 /* [RW] MCPB Channel x Stream Processor State Register 8 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_9        0x20a726b4 /* [RW] MCPB Channel x Stream Processor State Register 9 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_10       0x20a726b8 /* [RW] MCPB Channel x Stream Processor State Register 10 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_11       0x20a726bc /* [RW] MCPB Channel x Stream Processor State Register 11 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_12       0x20a726c0 /* [RW] MCPB Channel x Stream Processor State Register 12 */
#define BCHP_XPT_MCPB_CH13_SP_STATE_REG_13       0x20a726c4 /* [RW] MCPB Channel x Stream Processor State Register 13 */
#define BCHP_XPT_MCPB_CH13_DMA_BBUFF_CTRL        0x20a726c8 /* [RW] MCPB Channel x Burst buffer control */
#define BCHP_XPT_MCPB_CH13_DMA_BBUFF_CRC         0x20a726cc /* [RW] MCPB Channel x Current CRC value */
#define BCHP_XPT_MCPB_CH13_DMA_BBUFF0_RW_STATUS  0x20a726d0 /* [RW] MCPB Channel x Burst buffer 0 data specific information */
#define BCHP_XPT_MCPB_CH13_DMA_BBUFF0_RO_STATUS  0x20a726d4 /* [RW] MCPB Channel x Burst buffer 0 control specific information */
#define BCHP_XPT_MCPB_CH13_DMA_BBUFF1_RW_STATUS  0x20a726d8 /* [RW] MCPB Channel x Burst buffer 1 data specific information */
#define BCHP_XPT_MCPB_CH13_DMA_BBUFF1_RO_STATUS  0x20a726dc /* [RW] MCPB Channel x Burst buffer 1 control specific information */
#define BCHP_XPT_MCPB_CH13_TMEU_BLOCKOUT_CTRL    0x20a726e0 /* [RW] MCPB Channel x Blockout control information */
#define BCHP_XPT_MCPB_CH13_TMEU_NEXT_BO_MON      0x20a726e4 /* [RW] MCPB Channel x next Blockout monitor information */
#define BCHP_XPT_MCPB_CH13_TMEU_TIMING_CTRL      0x20a726e8 /* [RW] MCPB Channel x next Blockout monitor information */
#define BCHP_XPT_MCPB_CH13_TMEU_REF_DIFF_VALUE_TS_MBOX 0x20a726ec /* [RW] MCPB Channel x reference difference value and next Timestamp information */
#define BCHP_XPT_MCPB_CH13_TMEU_TS_ERR_BOUND_EARLY 0x20a726f0 /* [RW] MCPB Channel x TS error bound early information */
#define BCHP_XPT_MCPB_CH13_TMEU_TS_ERR_BOUND_LATE 0x20a726f4 /* [RW] MCPB Channel x TS error bound late information */
#define BCHP_XPT_MCPB_CH13_TMEU_NEXT_GPC_MON     0x20a726f8 /* [RW] MCPB Channel x next Global Pacing Counter and Timestamp monitor information */
#define BCHP_XPT_MCPB_CH13_TMEU_REF_DIFF_VALUE_SIGN 0x20a726fc /* [RW] MCPB Channel x reference difference value sign information */
#define BCHP_XPT_MCPB_CH13_TMEU_PES_PACING_CTRL  0x20a72700 /* [RW] MCPB Channel x PES pacing control information */
#define BCHP_XPT_MCPB_CH13_TMEU_SLOT_STATUS      0x20a72704 /* [RW] MCPB Channel x Slot 0 and Slot 1 information */
#define BCHP_XPT_MCPB_CH13_TMEU_TIMING_INFO_SLOT0_REG1 0x20a72708 /* [RW] MCPB Channel x timing information for Slot 0 */
#define BCHP_XPT_MCPB_CH13_TMEU_TIMING_INFO_SLOT0_REG2 0x20a7270c /* [RW] MCPB Channel x timing information for Slot 0 */
#define BCHP_XPT_MCPB_CH13_TMEU_TIMING_INFO_SLOT1_REG1 0x20a72710 /* [RW] MCPB Channel x timing information for Slot 1 */
#define BCHP_XPT_MCPB_CH13_TMEU_TIMING_INFO_SLOT1_REG2 0x20a72714 /* [RW] MCPB Channel x timing information for Slot 1 */
#define BCHP_XPT_MCPB_CH13_TMEU_TIMING_INFO_LAST_TIMESTAMP_DELTA 0x20a72718 /* [RW] MCPB Channel x last TS delta value */
#define BCHP_XPT_MCPB_CH13_TMEU_TIMING_INFO_LAST_NEXT_TIMESTAMP 0x20a7271c /* [RW] MCPB Channel x last NEXT TS value */
#define BCHP_XPT_MCPB_CH13_DCPM_STATUS           0x20a72720 /* [RW] MCPB Channel x DCPM status information */
#define BCHP_XPT_MCPB_CH13_DCPM_DESC_ADDR        0x20a72724 /* [RW] MCPB Channel x DCPM descriptor address information */
#define BCHP_XPT_MCPB_CH13_DCPM_DESC_DONE_INT_ADDR 0x20a72728 /* [RW] MCPB Channel x DCPM descriptor done interrupt address information */
#define BCHP_XPT_MCPB_CH13_DCPM_PAUSE_AFTER_GROUP_PACKETS_CTRL 0x20a7272c /* [RW] MCPB Channel x Pause after group of packets control information */
#define BCHP_XPT_MCPB_CH13_DCPM_PAUSE_AFTER_GROUP_PACKETS_PKT_COUNTER 0x20a72730 /* [RW] MCPB Channel x Pause after group of packets local packet counter */
#define BCHP_XPT_MCPB_CH13_DCPM_LOCAL_PACKET_COUNTER 0x20a72734 /* [RW] MCPB Channel x local packet counter */
#define BCHP_XPT_MCPB_CH13_DCPM_DATA_ADDR_UPPER  0x20a72738 /* [RW] MCPB Channel x DCPM data address information */
#define BCHP_XPT_MCPB_CH13_DCPM_DATA_ADDR_LOWER  0x20a7273c /* [RW] MCPB Channel x DCPM data address information */
#define BCHP_XPT_MCPB_CH13_DCPM_CURR_DESC_ADDR   0x20a72740 /* [RW] MCPB Channel x DCPM current descriptor address information */
#define BCHP_XPT_MCPB_CH13_DCPM_SLOT_STATUS      0x20a72744 /* [RW] MCPB Channel x DCPM slot status information */
#define BCHP_XPT_MCPB_CH13_DCPM_DESC_ADDR_SLOT_0 0x20a72748 /* [RW] MCPB Channel x DCPM completed slot 0 descriptor address information */
#define BCHP_XPT_MCPB_CH13_DCPM_DATA_ADDR_SLOT_0_UPPER 0x20a7274c /* [RW] MCPB Channel x DCPM completed slot 0 data address information */
#define BCHP_XPT_MCPB_CH13_DCPM_DATA_ADDR_SLOT_0_LOWER 0x20a72750 /* [RW] MCPB Channel x DCPM completed slot 0 data address information */
#define BCHP_XPT_MCPB_CH13_DCPM_DESC_ADDR_SLOT_1 0x20a72754 /* [RW] MCPB Channel x DCPM completed slot 1 descriptor address information */
#define BCHP_XPT_MCPB_CH13_DCPM_DATA_ADDR_SLOT_1_UPPER 0x20a72758 /* [RW] MCPB Channel x DCPM completed slot 1 data address information */
#define BCHP_XPT_MCPB_CH13_DCPM_DATA_ADDR_SLOT_1_LOWER 0x20a7275c /* [RW] MCPB Channel x DCPM completed slot 1 data address information */

#endif /* #ifndef BCHP_XPT_MCPB_CH13_H__ */

/* End of File */
