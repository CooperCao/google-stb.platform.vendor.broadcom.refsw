/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 * The launch point for all information concerning RDB is found at:
 *   http://bcgbu.broadcom.com/RDB/SitePages/Home.aspx
 *
 * Date:           Generated on               Thu Apr 13 10:09:32 2017
 *                 Full Compile MD5 Checksum  7f180d7646477bba2bae1a701efd9ef5
 *                     (minus title and desc)
 *                 MD5 Checksum               a2a4a53aa20c0c2f46073b879159b85d
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     1395
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   LOCAL tools/dvtsw/current/Linux/combo_header.pl
 *
 *
********************************************************************************/

#ifndef BCHP_XPT_MEMDMA_MCPB_CH1_H__
#define BCHP_XPT_MEMDMA_MCPB_CH1_H__

/***************************************************************************
 *XPT_MEMDMA_MCPB_CH1 - MCPB Channel 1 Configuration
 ***************************************************************************/
#define BCHP_XPT_MEMDMA_MCPB_CH1_RUN             0x02261400 /* [RW][32] MCPB Channel x RUN set/clear register */
#define BCHP_XPT_MEMDMA_MCPB_CH1_WAKE_SET        0x02261404 /* [RW][32] WAKE set register for MCPB Channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DATA_FLOW_CTRL  0x02261408 /* [RW][32] MCPB Channel x data flow controls */
#define BCHP_XPT_MEMDMA_MCPB_CH1_CPU_ID_GRAB_CHANNEL 0x0226140c /* [RW][32] MCPB Channel x Grab channel register */
#define BCHP_XPT_MEMDMA_MCPB_CH1_CPU_ID_CFG_DONE 0x02261410 /* [RW][32] MCPB Channel x Configuration Done register */
#define BCHP_XPT_MEMDMA_MCPB_CH1_CPU_ID_STATUS   0x02261414 /* [RO][32] MCPB Channel x CPU ID Status register */
#define BCHP_XPT_MEMDMA_MCPB_CH1_BTP_CPU_ID_STATUS 0x02261418 /* [RW][32] MCPB Channel x BTP CPU ID Status register */
#define BCHP_XPT_MEMDMA_MCPB_CH1_BUFF_DATA_RDY_SET 0x0226141c /* [RW][32] Set the BUFF_DATA_RDY bit for AV and Retransmission for MCPB Channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_RUN_INVALID     0x02261420 /* [RW][32] Set and Clear Run Invalid for MCPB Channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_CHAN_CTRL  0x02261424 /* [RW][32] Escheduler control for MCPB channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ETH_RETRANS_AV_PAUSE 0x02261428 /* [RW][32] Set and clear AV pause bit during retransmission for MCPB Channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_FLOW_CTRL  0x0226142c /* [RW][32] Flow control bits for Escheduler for MCPB Channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ETH_LAST_EPKT_FORCE_PUSH 0x02261430 /* [RW][32] Set and Clear host initiated push of partial epacket present in internal buffers for MCPB Channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_BTP0_CTRL1      0x02261434 /* [RW][32] BTP0 control register for MCPB Channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_BTP1_CTRL1      0x02261438 /* [RW][32] BTP1 control register for MCPB Channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_BTP2_CTRL1      0x0226143c /* [RW][32] BTP2 control register for MCPB Channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_BTP_STATUS0     0x02261440 /* [RO][32] BTP slot status register for channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_PARSER_BAND_ID_CTRL 0x02261444 /* [RW][32] Parser Band ID control for channel x */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_ADDR   0x02261448 /* [RW][64] MCPB Channel x Descriptor control information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_CURR_DESC_ADDR 0x02261450 /* [RW][64] MCPB Channel x Current Descriptor address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_NEXT_DESC_ADDR 0x02261458 /* [RW][64] MCPB Channel x Next Descriptor address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BUFF_BASE_ADDR 0x02261460 /* [RW][64] MCPB Channel x Data Buffer Base address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BUFF_END_ADDR 0x02261468 /* [RW][64] MCPB Channel x Data Buffer End address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BUFF_CURR_RD_ADDR 0x02261470 /* [RW][64] MCPB Channel x Current Data Buffer Read address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BUFF_WR_ADDR 0x02261478 /* [RW][64] MCPB Channel x Data Buffer Write address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DATA_CTRL   0x02261480 /* [RW][32] MCPB Channel x Data control information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_STATUS_0    0x02261488 /* [RW][32] MCPB Channel x Status information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_STATUS_1    0x02261490 /* [RW][32] MCPB Channel x CRC value */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_STATUS_2    0x02261498 /* [RW][32] MCPB Channel x Manual mode status */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT0_STATUS_0 0x022614a0 /* [RW][32] MCPB channel x Descriptor Slot 0 status information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT0_STATUS_1 0x022614a8 /* [RW][32] MCPB channel x Descriptor Slot 0 status information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT0_CURR_DESC_ADDR 0x022614b0 /* [RW][64] MCPB Channel x Descriptor Slot 0 Current Descriptor Address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT0_CURR_DATA_ADDR 0x022614b8 /* [RW][64] MCPB Channel x Descriptor Slot 0 Current Data Address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT0_NEXT_TIMESTAMP 0x022614c0 /* [RW][32] MCPB Channel x Descriptor Slot 0 Next Packet Timestamp */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT0_PKT2PKT_TIMESTAMP_DELTA 0x022614c8 /* [RW][32] MCPB Channel x Descriptor Slot 0 Packet to packet Timestamp delta */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT1_STATUS_0 0x022614d0 /* [RW][32] MCPB channel x Descriptor Slot 1 status information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT1_STATUS_1 0x022614d8 /* [RW][32] MCPB channel x Descriptor Slot 1 status information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT1_CURR_DESC_ADDR 0x022614e0 /* [RW][64] MCPB Channel x Descriptor Slot 1 Current Descriptor Address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT1_CURR_DATA_ADDR 0x022614e8 /* [RW][64] MCPB Channel x Descriptor Slot 1 Current Data Address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT1_NEXT_TIMESTAMP 0x022614f0 /* [RW][32] MCPB Channel x Descriptor Slot 1 Next Packet Timestamp */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_SLOT1_PKT2PKT_TIMESTAMP_DELTA 0x022614f8 /* [RW][32] MCPB Channel x Descriptor Slot 1 Packet to packet Timestamp delta */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_RETRANS_BUFF_BASE_ADDR 0x02261500 /* [RW][64] MCPB Channel x Retransmission Data Buffer Base address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_RETRANS_BUFF_END_ADDR 0x02261508 /* [RW][64] MCPB Channel x Retransmission Data Buffer End address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_RETRANS_BUFF_CURR_RD_ADDR 0x02261510 /* [RW][64] MCPB Channel x Current Retransmission Data Buffer Read address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_RETRANS_BUFF_WR_ADDR 0x02261518 /* [RW][64] MCPB Channel x Retransmission Data Buffer Write address */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_RETRANS_BUFF_SP_CURR_DATA_ADDR 0x02261520 /* [RW][64] MCPB Channel x Retransmission Current Data Address for SP */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_RETRANS_DATA_CTRL 0x02261528 /* [RW][32] MCPB Channel x Retransmission Data Request controls */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_PKT_LEN      0x02261530 /* [RW][32] MCPB Channel x Packet length control */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_PARSER_CTRL  0x02261538 /* [RW][32] MCPB Channel x Parser control */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_PARSER_CTRL1 0x02261540 /* [RW][32] MCPB Channel x Parser control 1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_TS_CONFIG    0x02261548 /* [RW][32] MCPB Channel x TS Configuration */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_PES_ES_CONFIG 0x02261550 /* [RW][32] MCPB Channel x PES and ES Configuration */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_PES_SYNC_COUNTER 0x02261558 /* [RW][32] MCPB Channel x PES Sync counter */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_RESERVED     0x02261560 /* [RW][32] MCPB Channel x reserved */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_0  0x02261568 /* [RW][32] MCPB Channel x Stream Processor State Register 0 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_1  0x02261570 /* [RW][32] MCPB Channel x Stream Processor State Register 1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_2  0x02261578 /* [RW][32] MCPB Channel x Stream Processor State Register 2 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_3  0x02261580 /* [RW][32] MCPB Channel x Stream Processor State Register 3 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_4  0x02261588 /* [RW][32] MCPB Channel x Stream Processor State Register 4 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_5  0x02261590 /* [RW][32] MCPB Channel x Stream Processor State Register 5 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_6  0x02261598 /* [RW][32] MCPB Channel x Stream Processor State Register 6 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_7  0x022615a0 /* [RW][32] MCPB Channel x Stream Processor State Register 7 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_8  0x022615a8 /* [RW][32] MCPB Channel x Stream Processor State Register 8 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_9  0x022615b0 /* [RW][32] MCPB Channel x Stream Processor State Register 9 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_10 0x022615b8 /* [RW][32] MCPB Channel x Stream Processor State Register 10 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_11 0x022615c0 /* [RW][32] MCPB Channel x Stream Processor State Register 11 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_12 0x022615c8 /* [RW][32] MCPB Channel x Stream Processor State Register 12 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_SP_STATE_REG_13 0x022615d0 /* [RW][32] MCPB Channel x Stream Processor State Register 13 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BBUFF_CTRL  0x022615d8 /* [RW][32] MCPB Channel x Burst buffer control */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BBUFF_CRC   0x022615dc /* [RW][32] MCPB Channel x Current CRC value */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DATA_BUFF_DEPTH_MONITOR_COMMIT 0x022615e0 /* [RW][32] MCPB Channel x Data buffer depth monitor commit register */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DATA_BUFF_DEPTH_MONITOR 0x022615e4 /* [RW][32] MCPB Channel x Data buffer depth monitor */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BBUFF0_INTERNAL_CTRL1 0x022615e8 /* [RW][32] MCPB Channel x Burst buffer 0 data specific information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BBUFF0_INTERNAL_CTRL2 0x022615ec /* [RW][32] MCPB Channel x Burst buffer 0 control specific information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BBUFF1_INTERNAL_CTRL1 0x022615f0 /* [RW][32] MCPB Channel x Burst buffer 1 data specific information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BBUFF1_INTERNAL_CTRL2 0x022615f4 /* [RW][32] MCPB Channel x Burst buffer 1 control specific information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BBUFF2_INTERNAL_CTRL1 0x022615f8 /* [RW][32] MCPB Channel x Burst buffer 2 data specific information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_BBUFF2_INTERNAL_CTRL2 0x022615fc /* [RW][32] MCPB Channel x Burst buffer 2 control specific information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_BLOCKOUT_CTRL 0x02261600 /* [RW][32] MCPB Channel x Blockout control information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_NEXT_BO_MON 0x02261604 /* [RW][32] MCPB Channel x next Blockout monitor information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_TIMING_CTRL 0x02261608 /* [RW][32] MCPB Channel x next Blockout monitor information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_REF_DIFF_VALUE_TS_MBOX 0x0226160c /* [RW][32] MCPB Channel x reference difference value and next Timestamp information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_TS_ERR_BOUND_EARLY 0x02261610 /* [RW][32] MCPB Channel x TS error bound early information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_TS_ERR_BOUND_LATE 0x02261614 /* [RW][32] MCPB Channel x TS error bound late information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_NEXT_GPC_MON 0x02261618 /* [RW][32] MCPB Channel x next Global Pacing Counter and Timestamp monitor information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_REF_DIFF_VALUE_SIGN 0x0226161c /* [RW][32] MCPB Channel x reference difference value sign information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_PES_PACING_CTRL 0x02261620 /* [RW][32] MCPB Channel x PES pacing control information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_SLOT_STATUS 0x02261624 /* [RW][32] MCPB Channel x Slot 0 and Slot 1 information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_TIMING_INFO_SLOT0_REG1 0x02261628 /* [RW][32] MCPB Channel x timing information for Slot 0 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_TIMING_INFO_SLOT0_REG2 0x0226162c /* [RW][32] MCPB Channel x timing information for Slot 0 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_TIMING_INFO_SLOT1_REG1 0x02261630 /* [RW][32] MCPB Channel x timing information for Slot 1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_TIMING_INFO_SLOT1_REG2 0x02261634 /* [RW][32] MCPB Channel x timing information for Slot 1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_TIMING_INFO_LAST_TIMESTAMP_DELTA 0x02261638 /* [RW][32] MCPB Channel x last TS delta value */
#define BCHP_XPT_MEMDMA_MCPB_CH1_TMEU_TIMING_INFO_LAST_NEXT_TIMESTAMP 0x0226163c /* [RW][32] MCPB Channel x last NEXT TS value */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_STATUS     0x02261640 /* [RW][32] MCPB Channel x DCPM status information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_DESC_ADDR  0x02261648 /* [RW][64] MCPB Channel x DCPM descriptor address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_DESC_DONE_INT_ADDR 0x02261650 /* [RW][64] MCPB Channel x DCPM descriptor done interrupt address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_PAUSE_AFTER_GROUP_PACKETS_CTRL 0x02261658 /* [RW][32] MCPB Channel x Pause after group of packets control information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_PAUSE_AFTER_GROUP_PACKETS_PKT_COUNTER 0x02261660 /* [RW][32] MCPB Channel x Pause after group of packets local packet counter */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_LOCAL_PACKET_COUNTER 0x02261668 /* [RW][32] MCPB Channel x local packet counter */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_PA1_DATA_ADDR 0x02261670 /* [RW][64] MCPB Channel x DCPM PA1 data address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_PA2_DATA_ADDR 0x02261678 /* [RW][64] MCPB Channel x DCPM PA2 data address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_DATA_ADDR  0x02261680 /* [RW][64] MCPB Channel x DCPM data address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_CURR_DESC_ADDR 0x02261688 /* [RW][64] MCPB Channel x DCPM current descriptor address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_RETRANS_DATA_ADDR 0x02261690 /* [RW][64] MCPB Channel x DCPM retransmission data address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_SLOT_STATUS 0x02261698 /* [RW][32] MCPB Channel x DCPM slot status information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_DESC_ADDR_SLOT_0 0x022616a0 /* [RW][64] MCPB Channel x DCPM completed slot 0 descriptor address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_DATA_ADDR_SLOT_0 0x022616a8 /* [RW][64] MCPB Channel x DCPM completed slot 0 data address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_RETRANS_DATA_ADDR_SLOT_0 0x022616b0 /* [RW][64] MCPB Channel x DCPM completed slot 0 data address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_DESC_ADDR_SLOT_1 0x022616b8 /* [RW][64] MCPB Channel x DCPM completed slot 1 descriptor address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_DATA_ADDR_SLOT_1 0x022616c0 /* [RW][64] MCPB Channel x DCPM completed slot 1 data address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_DCPM_RETRANS_DATA_ADDR_SLOT_1 0x022616c8 /* [RW][64] MCPB Channel x DCPM completed slot 1 retransmission data address information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ON_CHIP_DESC_FIFO_BASE_POINTER 0x022616d0 /* [RW][32] MCPB Channel x On chip descriptor fifo Base pointer information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ON_CHIP_DESC_FIFO_END_POINTER 0x022616d4 /* [RW][32] MCPB Channel x On chip descriptor fifo Base pointer information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ON_CHIP_DESC_FIFO_RD_POINTER 0x022616d8 /* [RW][32] MCPB Channel x On chip descriptor fifo Read pointer information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ON_CHIP_DESC_FIFO_WR_POINTER 0x022616dc /* [RW][32] MCPB Channel x On chip descriptor fifo Write pointer information */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP0_CTRL3 0x022616e0 /* [RW][32] BTP0 control registers */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP0_SLOT0_CTRL4 0x022616e4 /* [RW][32] BTP0 control registers for slot 0 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP0_SLOT1_CTRL5 0x022616e8 /* [RW][32] BTP0 control registers for slot 1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP0_SLOT0_CTRL6 0x022616ec /* [RW][32] BTP0 insertion interval value for slot 0 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP0_SLOT1_CTRL7 0x022616f0 /* [RW][32] BTP0 insertion interval value for slot 1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP1_CTRL3 0x022616f4 /* [RW][32] BTP1 control registers */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP1_SLOT0_CTRL4 0x022616f8 /* [RW][32] BTP1 control registers for slot 0 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP1_SLOT1_CTRL5 0x022616fc /* [RW][32] BTP1 control registers for slot 1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP1_SLOT0_CTRL6 0x02261700 /* [RW][32] BTP1 insertion interval value for slot 0 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP1_SLOT1_CTRL7 0x02261704 /* [RW][32] BTP1 insertion interval value for slot 1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP2_CTRL3 0x02261708 /* [RW][32] BTP2 control registers */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP2_SLOT0_CTRL4 0x0226170c /* [RW][32] BTP2 control registers for slot 0 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP2_SLOT1_CTRL5 0x02261710 /* [RW][32] BTP2 control registers for slot 1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP2_SLOT0_CTRL6 0x02261714 /* [RW][32] BTP2 insertion interval value for slot 0 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP2_SLOT1_CTRL7 0x02261718 /* [RW][32] BTP2 insertion interval value for slot 1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP0_STATUS1 0x0226171c /* [RW][32] Current value of byte/packet counter for BTP0 slot 0/1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP1_STATUS1 0x02261720 /* [RW][32] Current value of byte/packet counter for BTP1 slot 0/1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_BTP2_STATUS1 0x02261724 /* [RW][32] Current value of byte/packet counter for BTP2 slot 0/1 */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_TCPIP_MAX_SEQUENCE_NUMBER 0x02261728 /* [RW][32] Max sequence number for TCPIP depth calculations */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_TCPIP_CURR_SEQUENCE_NUMBER 0x0226172c /* [RW][32] Max sequence number for TCPIP depth calculations */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_TCPIP_WINDOW_THRESHOLD 0x02261730 /* [RW][32] Threshold value for TCPIP windowing check */
#define BCHP_XPT_MEMDMA_MCPB_CH1_ESCD_ACH_CTRL   0x02261734 /* [RW][32] Control signals in ACH for queue management */

#endif /* #ifndef BCHP_XPT_MEMDMA_MCPB_CH1_H__ */

/* End of File */
