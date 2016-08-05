/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*****************************************************************************/


#ifndef PCIE_DMA_H
#define PCIE_DMA_H

#define PCIE_DMA_TX_FIRST_DESC_L_ADDR_LIST0 0xF0464400 //Mask last 5 bits for DescAddr
#define PCIE_DMA_TX_FIRST_DESC_U_ADDR_LIST0 0xF0464404

#define PCIE_DMA_TX_FIRST_DESC_L_ADDR_LIST1 0xF0464408 //Mask last 5 bits for DescAddr
#define PCIE_DMA_TX_FIRST_DESC_U_ADDR_LIST1 0xF046440C

#define PCIE_DMA_TX_SW_DESC_LIST_CTRL_STS   0xF0464410
#define PCIE_DMA_TX_WAKE_CTRL 		    0xF0464414

#define PCIE_DMA_TX_ERROR_STATUS 	    0x10414418
#define PCIE_DMA_TX_LIST0_CUR_DESC_L_ADDR   0xF046441C
#define PCIE_DMA_TX_LIST0_CUR_DESC_U_ADDR   0xF0464420
#define PCIE_DMA_TX_LIST0_CUR_BYTE_CNT	    0xF0464424

#define PCIE_DMA_TX_LIST1_CUR_DESC_L_ADDR   0xF0464428
#define PCIE_DMA_TX_LIST1_CUR_DESC_U_ADDR   0xF046442C
#define PCIE_DMA_TX_LIST1_CUR_BYTE_CNT      0xF0464430

#define PCIE_DMA_RX_LIST0_CUR_DESC_L_ADDR   0xF0464450
#define PCIE_DMA_RX_FIRST_DESC_L_ADDR_LIST0 0xF0464434
#define PCIE_DMA_RX_SW_DESC_LIST_CTRL_STS   0xF0464444
#define PCIE_DMA_RX_WAKE_CTRL		    0xF0464448

// simplified calls for each volatile address area

//#define pcie_dma_desc_base *((volatile uint32 *)(PCIE_DMA_DESC_BASE))
#define pcie_dma_tx_first_desc_l_addr_list0 *((volatile uint32 *)(PCIE_DMA_TX_FIRST_DESC_L_ADDR_LIST0))
#define pcie_dma_tx_first_desc_u_addr_list0 *((volatile uint32 *)(PCIE_DMA_TX_FIRST_DESC_U_ADDR_LIST0))

#define pcie_dma_tx_first_desc_l_addr_list1 *((volatile uint32 *)(PCIE_DMA_TX_FIRST_DESC_L_ADDR_LIST1))
#define pcie_dma_tx_first_desc_u_addr_list1 *((volatile uint32 *)(PCIE_DMA_TX_FIRST_DESC_U_ADDR_LIST1))

#define pcie_dma_tx_sw_desc_list_ctrl_sts *((volatile uint32 *)(PCIE_DMA_TX_SW_DESC_LIST_CTRL_STS))
#define pcie_dma_tx_wake_ctrl *((volatile uint32 *)(PCIE_DMA_TX_WAKE_CTRL))
#define pcie_dma_tx_error_status *((volatile uint32 *)(PCIE_DMA_TX_ERROR_STATUS))

#define pcie_dma_tx_cur_desc_l_addr_list0 *((volatile uint32 *)(PCIE_DMA_TX_LIST0_CUR_DESC_L_ADDR))
#define pcie_dma_tx_cur_desc_u_addr_list0 *((volatile uint32 *)(PCIE_DMA_TX_LIST0_CUR_DESC_U_ADDR))
#define pcie_dma_tx_cur_byte_cnt_list0    *((volatile uint32 *)(PCIE_DMA_TX_LIST0_CUR_BYTE_CNT))

#define pcie_dma_tx_cur_desc_l_addr_list1 *((volatile uint32 *)(PCIE_DMA_TX_LIST1_CUR_DESC_L_ADDR))
#define pcie_dma_tx_cur_desc_u_addr_list1 *((volatile uint32 *)(PCIE_DMA_TX_LIST1_CUR_DESC_U_ADDR))
#define pcie_dma_tx_cur_byte_cnt_list1 	  *((volatile uint32 *)(PCIE_DMA_TX_LIST1_CUR_BYTE_CNT))

#define pcie_dma_rx_cur_desc_l_addr_list0 *((volatile uint32 *)(PCIE_DMA_RX_LIST0_CUR_DESC_L_ADDR))
#define pcie_dma_rx_sw_desc_list_ctrl_sts *((volatile uint32 *)(PCIE_DMA_RX_SW_DESC_LIST_CTRL_STS))
#define pcie_dma_rx_wake_ctrl *((volatile uint32 *)(PCIE_DMA_RX_WAKE_CTRL))
#define pcie_dma_rx_first_desc_l_addr_list0 *((volatile uint32 *)(PCIE_DMA_RX_FIRST_DESC_L_ADDR_LIST0))

// Other defines that can be modified
#define DESC_SIZE 256

typedef struct
{


	uint32 fMemAddr;
	uint32 fPcieAddrLo;
	uint32 fPcieAddrHi;
	uint32 fWord3;
	uint32 fWord4;
	uint32 fNextDescAddrLo;	// 4:0 must be written 0
	uint32 fNextDescAddrHi;
	uint32 fReserved;

} DmaDescRegs;

enum DmaDescRegMasks{

	//word3
	kInterruptEn = 31,
	kTransferSize = 2,

	//word4
	kLast = 31,
	kDir = 30,
	kCont = 2,
	kEndian = 0
};

enum DmaDescCtlStat{

	kDescEndian	= 12,
	kLocalDesc	= 9,	// 0 = descriptors in PCIE space, 1 = descriptors in local mem
	kDmaMode	= 8,	// 0 = ping pong, 1 = wake/resume
	kDmaStatus      = 4,		// 00 = idle, 01 = busy, 10 = sleep, 11 = error
	kDmaError	= 3,
	kDmaBusy	= 1,
	kDma_data_serv_ptr = 3,	// DMA data state machine servicing List 0 or List 1
	kDesc_serv_ptr	= 2,		// descriptor state machine is servicing List 0 or List1
	kRunStop	= 0		// 1 = Run, 0 = Stop

};

enum DmaWakeCtl{

	kDmaWakeMode = 0x2,
	kDmaWake     = 0x1
};

enum DmaCurDesc{

	kCurrAddrLo 	= 0xFFFFFC0,
	kCurList    	= 0x08,
	kCurProgress	= 0x04,
	kCurStat	= 0x03
};


//volatile uint32 PCIE_DMA_DESC_BASE; // PUT IN ADDR HERE
//volatile DmaDescRegs *pDmaDescRegs = ( volatile DmaDescRegs *) PCIE_DMA_DESC_BASE;

#endif
