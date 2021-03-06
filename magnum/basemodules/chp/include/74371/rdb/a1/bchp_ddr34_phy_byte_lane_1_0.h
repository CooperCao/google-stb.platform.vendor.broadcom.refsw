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
 * Date:           Generated on               Fri Feb 20 00:05:25 2015
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

#ifndef BCHP_DDR34_PHY_BYTE_LANE_1_0_H__
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_H__

/***************************************************************************
 *DDR34_PHY_BYTE_LANE_1_0 - DDR34 Byte Lane #1 control registers
 ***************************************************************************/
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DQS_P 0x00906600 /* [RW] Write channel DQS-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DQS_N 0x00906604 /* [RW] Write channel DQS-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DQ0 0x00906608 /* [RW] Write channel DQ0 VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DQ1 0x0090660c /* [RW] Write channel DQ1 VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DQ2 0x00906610 /* [RW] Write channel DQ2 VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DQ3 0x00906614 /* [RW] Write channel DQ3 VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DQ4 0x00906618 /* [RW] Write channel DQ4 VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DQ5 0x0090661c /* [RW] Write channel DQ5 VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DQ6 0x00906620 /* [RW] Write channel DQ6 VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DQ7 0x00906624 /* [RW] Write channel DQ7 VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_DM 0x00906628 /* [RW] Write channel DM VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_WR_EDC 0x0090662c /* [RW] Write channel EDC VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQSP 0x00906630 /* [RW] Read channel DQSP VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQSN 0x00906634 /* [RW] Read channel DQSP VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ0P 0x00906638 /* [RW] Read channel DQ0-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ0N 0x0090663c /* [RW] Read channel DQ0-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ1P 0x00906640 /* [RW] Read channel DQ1-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ1N 0x00906644 /* [RW] Read channel DQ1-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ2P 0x00906648 /* [RW] Read channel DQ2-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ2N 0x0090664c /* [RW] Read channel DQ2-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ3P 0x00906650 /* [RW] Read channel DQ3-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ3N 0x00906654 /* [RW] Read channel DQ3-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ4P 0x00906658 /* [RW] Read channel DQ4-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ4N 0x0090665c /* [RW] Read channel DQ4-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ5P 0x00906660 /* [RW] Read channel DQ5-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ5N 0x00906664 /* [RW] Read channel DQ5-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ6P 0x00906668 /* [RW] Read channel DQ6-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ6N 0x0090666c /* [RW] Read channel DQ6-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ7P 0x00906670 /* [RW] Read channel DQ7-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DQ7N 0x00906674 /* [RW] Read channel DQ7-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DMP 0x00906678 /* [RW] Read channel DM-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_DMN 0x0090667c /* [RW] Read channel DM-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_EDCP 0x00906680 /* [RW] Read channel EDC-P VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_EDCN 0x00906684 /* [RW] Read channel EDC-N VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_EN_CS0 0x00906688 /* [RW] Read channel CS_N[0] read enable VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CONTROL_RD_EN_CS1 0x0090668c /* [RW] Read channel CS_N[1] read enable VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_CLK_CONTROL 0x00906690 /* [RW] DDR interface signal Write Leveling CLK VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_VDL_LDE_CONTROL 0x00906694 /* [RW] DDR interface signal Write Leveling Capture Enable VDL control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_RD_EN_DLY_CYC 0x009066a0 /* [RW] Read enable bit-clock cycle delay control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_WR_CHAN_DLY_CYC 0x009066a4 /* [RW] Write leveling bit-clock cycle delay control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_READ_CONTROL 0x009066b0 /* [RW] Read channel datapath control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_READ_FIFO_ADDR 0x009066b4 /* [RW] Read fifo addresss pointer register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_READ_FIFO_DATA 0x009066b8 /* [RO] Read fifo data register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_READ_FIFO_DM_DBI 0x009066bc /* [RO] Read fifo dm/dbi register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_READ_FIFO_STATUS 0x009066c0 /* [RO] Read fifo status register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_READ_FIFO_CLEAR 0x009066c4 /* [WO] Read fifo status clear register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_IDLE_PAD_CONTROL 0x009066c8 /* [RW] Idle mode SSTL pad control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_DRIVE_PAD_CTL 0x009066cc /* [RW] SSTL pad drive characteristics control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_RD_EN_DRIVE_PAD_CTL 0x009066d0 /* [RW] SSTL read enable pad drive characteristics control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_STATIC_PAD_CTL 0x009066d4 /* [RW] pad rx and tx characteristics control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_WR_PREAMBLE_MODE 0x009066d8 /* [RW] Write cycle preamble control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_ODT_CONTROL 0x009066e0 /* [RW] Read channel ODT control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_EDC_DPD_CONTROL 0x009066f0 /* [RW] GDDR5M EDC digital phase detector control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_EDC_DPD_STATUS 0x009066f4 /* [RO] GDDR5M EDC digital phase detector status register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_EDC_DPD_OUT_CONTROL 0x009066f8 /* [RW] GDDR5M EDC digital phase detector output signal control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_EDC_DPD_OUT_STATUS 0x009066fc /* [RO] GDDR5M EDC digital phase detector output signal status register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_EDC_DPD_OUT_STATUS_CLEAR 0x00906700 /* [WO] GDDR5M EDC digital phase detector output signal status clear register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_EDC_CRC_CONTROL 0x00906704 /* [RW] GDDR5M EDC signal path CRC control register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_EDC_CRC_STATUS 0x00906708 /* [RO] GDDR5M EDC signal path CRC status register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_EDC_CRC_COUNT 0x0090670c /* [RO] GDDR5M EDC signal path CRC counter register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_EDC_CRC_STATUS_CLEAR 0x00906710 /* [WO] GDDR5M EDC signal path CRC counter register */
#define BCHP_DDR34_PHY_BYTE_LANE_1_0_BL_SPARE_REG 0x00906714 /* [RW] Byte-Lane Spare register */

#endif /* #ifndef BCHP_DDR34_PHY_BYTE_LANE_1_0_H__ */

/* End of File */
