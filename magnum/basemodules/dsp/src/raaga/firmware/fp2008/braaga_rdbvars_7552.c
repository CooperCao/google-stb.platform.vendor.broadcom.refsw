/***************************************************************************
 *     Copyright (c) 1999-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************/

#include "bchp.h"
const uint32_t BDSP_IMG_system_rdbvars_array1[] = {
	0x00020000, /* Imem Size */
	0x00018000, /* Dmem Size */
	0x00000031, /* Uart baud rate div factor */
	0x165A0BC0, /* Firepath frequency in MHz */
	0x80021080, /* RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS 		- DSP Subsystem UART Status */
	0x80021084, /* RAAGA_DSP_PERI_DBG_CTRL_UART_RCV_DATA 	- DSP Subsystem UART Receive Data */
	0x80021088, /* RAAGA_DSP_PERI_DBG_CTRL_UART_XMIT_DATA 	- DSP Subsystem UART Transmit Data */
	0x8002108c, /* RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL 		- DSP Subsystem UART Control */
	0x80021000, /* RAAGA_DSP_TIMERS_TSM_TIMER 				- TSM Timer Control Register */
	0x80021004, /* RAAGA_DSP_TIMERS_TSM_TIMER_VALUE 		- Time of the TSM Timer */
	0x80023000, /* FIFO 0 BASEADDRESS */
	0x80023004, /* FIFO ID Containing HOST2DSP Command */
	0x80021140, /* RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_0 		- SW SET CLEAR Status Register (Register 0) */
	0x80021144, /* RAAGA_DSP_PERI_SW_MSG_BITS_SET_0 		- SW SET CLEAR Set Register (Register 0) */
	0x80021148, /* RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_0 		- SW SET CLEAR Clear Register (Register 0) */
	0x8002114c, /* RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_1 		- SW SET CLEAR Status Register (Register 1) */
	0x80021150, /* RAAGA_DSP_PERI_SW_MSG_BITS_SET_1 		- SW SET CLEAR Set Register (Register 1) */
	0x80021154, /* RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_1 		- SW SET CLEAR Clear Register (Register 1) */
	0x80022404, /* RAAGA_DSP_FW_INTH_HOST_SET 				- Host Interrupt Set Register */
	0x80021054, /* RAAGA_DSP_TIMERS_WATCHDOG_TIMER 			- Watchdog Timer Control Register */
	0x80021058, /* RAAGA_DSP_TIMERS_WATCHDOG_TIMER_VALUE 	- Time of the Watchdog Timer */
	0x80034000, /* RAAGA_DSP_MEM_SUBSYSTEM_MDMEMRAM0 		- MDMEM RAM */
	0x800233c0, /* Base Address for FIFO DRAMLOGS */
	0x800233c4, /* End Address for FIFO DRAMLOGS */
	0x800233c8, /* Write Pointer for FIFO DRAMLOGS */
	0x800233cc, /* Read Pointer for FIFO DRAMLOGS */
	0x80038000, /* RAAGA_DSP_MEM_SUBSYSTEM_VOMRAM0 			- VOM RAM */
	0x80020000, /* RAAGA_DSP_MISC_REVISION Audio 			- DSP System Revision Register */
	0x80021200, /* RAAGA_DSP_DMA_QUEUE_PRIORITY 			- Priority of DMA Queues */
	0x80021120, /* RAAGA_DSP_PERI_SW_MAILBOX0 				- Mailbox Register 0 */
	0x80021124, /* RAAGA_DSP_PERI_SW_MAILBOX1 				- Mailbox Register 1 */
	0x80021128, /* RAAGA_DSP_PERI_SW_MAILBOX2 				- Mailbox Register 2 */
	0x8002112c, /* RAAGA_DSP_PERI_SW_MAILBOX3 				- Mailbox Register 3 */
	0x80021130, /* RAAGA_DSP_PERI_SW_MAILBOX4 				- Mailbox Register 4 */
	0x80021134, /* RAAGA_DSP_PERI_SW_MAILBOX5 				- Mailbox Register 5 */
	0x80021138, /* RAAGA_DSP_PERI_SW_MAILBOX6 				- Mailbox Register 6 */
	0x8002113c, /* RAAGA_DSP_PERI_SW_MAILBOX7 				- Mailbox Register 7 */
	0x80023500, /* RAAGA_DSP_FW_CFG_SW_UNDEFINED_SPARE0 	- Sw Undefined spare 0 */
	0x80021248, /*  RAAGA_DSP_DMA_SRC_ADDR_Q0 */
	0x80021288, /*  RAAGA_DSP_DMA_SRC_ADDR_Q1 */
	0x8002124c, /*  RAAGA_DSP_DMA_DEST_ADDR_Q0 */
	0x80021244, /*  RAAGA_DSP_DMA_SCB_GEN_CMD_TYPE_Q0 */
	0x80021250, /*  RAAGA_DSP_DMA_TRANSFER_Q0 */
	0x80022214, /*  RAAGA_DSP_INTH_HOST_MASK_CLEAR */
	0x80022210, /*  RAAGA_DSP_INTH_HOST_MASK_SET */
	0x80022204, /*  RAAGA_DSP_INTH_HOST_SET */
	0x8002125c, /*  RAAGA_DSP_DMA_TOKEN_ID_CLR_Q0 */
	0x80023010, /* Base Address for FIFO 0 */
	0x80023014, /* End Address for FIFO 0 */
	0x80023018, /* Write Pointer for FIFO 0 */
	0x8002301c, /* Read Pointer for FIFO 0 */
	0x80023020, /* Base Address for FIFO 1 */
    0x00000000, /* RAAGA_DSP_DMA_SRC_ADDR_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_DEST_ADDR_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_TRANSFER_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_PROGRESS_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_TOKEN_ID_CLR_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_MAX_SCB_BURST_SIZE_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_SCB_GEN_CMD_TYPE_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_DMA_ADDR1_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_DMA_ADDR2_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_VIDEO_PATCH_PARAM1_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_VIDEO_PATCH_PARAM2_VQ4 */
	0x00000000, /* RAAGA_DSP_DMA_SRC_ADDR_VQ5 */
	0x00000000, /* RAAGA_DSP_DMA_DRAM_VIDEO_NMBY0 */
	0x00000000, /* RAAGA_DSP_DMA_DRAM_VIDEO_BASE_ADDR0 */
	0x00000000, /* RAAGA_DSP_DMA_SCB_IF_CONFIG */
	0x10360124, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_REGISTER */
	0x00000002, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_MASK */
	0x00000001, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_SHIFT */
	0x10360124, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_REGISTER */
	0x00000001, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_MASK */
	0x00000000, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_SHIFT */
	0x00000007, /* NUM_MEM_PROTECT_REGIONS */
	0x00000000, /* MEM_PROTECT_REGION0_START */
	0x107fffff, /* MEM_PROTECT_REGION0_END */
	0x10900000, /* MEM_PROTECT_REGION1_START */
	0x10a0ffff, /* MEM_PROTECT_REGION1_END */
	0x10a1a6a0, /* MEM_PROTECT_REGION2_START */
	0x10a1afff, /* MEM_PROTECT_REGION2_END */
	0x10a1c000, /* MEM_PROTECT_REGION3_START */
	0x3fffffff, /* MEM_PROTECT_REGION3_END */
	0x40020000, /* MEM_PROTECT_REGION4_START */
	0x4fffffff, /* MEM_PROTECT_REGION4_END */
	0x50080000, /* MEM_PROTECT_REGION5_START */
	0x7fffffff, /* MEM_PROTECT_REGION5_END */
	0x82000000, /* MEM_PROTECT_REGION6_START */
	0xffffffff, /* MEM_PROTECT_REGION6_END */
	0x00000000, /* MEM_PROTECT_REGION7_START */
	0x00000000, /* MEM_PROTECT_REGION7_END */
	0x00000000, /* MEM_PROTECT_REGION8_START */
	0x00000000, /* MEM_PROTECT_REGION8_END */
	0x00000000, /* MEM_PROTECT_REGION9_START */
	0x00000000, /* MEM_PROTECT_REGION9_END */
	0x10404008, /* SUN_TOP_CTRL_BSP_FEATURE_TABLE_ADDR */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
	0x00000000, /* Not Used */
};
const uint32_t BDSP_IMG_system_rdbvars_header [2] = {sizeof(BDSP_IMG_system_rdbvars_array1), 1};
const void * const BDSP_IMG_system_rdbvars [2] = {BDSP_IMG_system_rdbvars_header, BDSP_IMG_system_rdbvars_array1};
/* End of File */
