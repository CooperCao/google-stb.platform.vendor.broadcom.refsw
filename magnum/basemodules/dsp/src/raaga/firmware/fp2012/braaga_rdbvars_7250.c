/***************************************************************************
 *     Copyright (c) 1999-2014, Broadcom Corporation
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
	0x00030000, /* IMEM_SIZE */
	0x00020000, /* DMEM_SIZE */
	0x000000E8, /* UART_BAUD_RATE_DIV_FACTOR */
	0x269FB200, /* DSP_FREQUENCY */
	0x80021080, /* RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS */
	0x80021084, /* RAAGA_DSP_PERI_DBG_CTRL_UART_RCV_DATA */
	0x80021088, /* RAAGA_DSP_PERI_DBG_CTRL_UART_XMIT_DATA */
	0x8002108c, /* RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL */
	0x80021000, /* RAAGA_DSP_TIMERS_TSM_TIMER */
	0x80021004, /* RAAGA_DSP_TIMERS_TSM_TIMER_VALUE */
	0x80023000, /* FIFO 0 BASEADDRESS */
	0x80023004, /* FIFO ID Containing HOST2DSP Command */
	0x80021140, /* RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_0 */
	0x80021144, /* RAAGA_DSP_PERI_SW_MSG_BITS_SET_0 */
	0x80021148, /* RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_0 */
	0x8002114c, /* RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_1 */
	0x80021150, /* RAAGA_DSP_PERI_SW_MSG_BITS_SET_1 */
	0x80021154, /* RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_1 */
	0x80022804, /* RAAGA_DSP_FW_INTH_HOST_SET */
	0x80021054, /* RAAGA_DSP_TIMERS_WATCHDOG_TIMER */
	0x80021058, /* RAAGA_DSP_TIMERS_WATCHDOG_TIMER_VALUE */
	0x80034000, /* RAAGA_DSP_MEM_SUBSYSTEM_MDMEMRAM0 */
	0x800233c0, /* Base Address for FIFO DRAMLOGS */
	0x800233c4, /* End Address for FIFO DRAMLOGS */
	0x800233c8, /* Write Pointer for FIFO DRAMLOGS */
	0x800233cc, /* Read Pointer for FIFO DRAMLOGS */
	0x80038000, /* RAAGA_DSP_MEM_SUBSYSTEM_VOMRAM0 */
	0x80020000, /* RAAGA_DSP_MISC_REVISION */
	0x80021400, /* RAAGA_DSP_DMA_QUEUE_PRIORITY */
	0x80021120, /* RAAGA_DSP_PERI_SW_MAILBOX0 */
	0x80021124, /* RAAGA_DSP_PERI_SW_MAILBOX1 */
	0x80021128, /* RAAGA_DSP_PERI_SW_MAILBOX2 */
	0x8002112c, /* RAAGA_DSP_PERI_SW_MAILBOX3 */
	0x80021130, /* RAAGA_DSP_PERI_SW_MAILBOX4 */
	0x80021134, /* RAAGA_DSP_PERI_SW_MAILBOX5 */
	0x80021138, /* RAAGA_DSP_PERI_SW_MAILBOX6 */
	0x8002113c, /* RAAGA_DSP_PERI_SW_MAILBOX7 */
	0x80023500, /* RAAGA_DSP_FW_CFG_SW_UNDEFINED_SPARE0 */
	0x80021448, /* RAAGA_DSP_DMA_SRC_ADDR_Q0 */
	0x80021488, /* RAAGA_DSP_DMA_SRC_ADDR_Q1 */
	0x8002144c, /* RAAGA_DSP_DMA_DEST_ADDR_Q0 */
	0x80021444, /* RAAGA_DSP_DMA_SCB_GEN_CMD_TYPE_Q0 */
	0x80021450, /* RAAGA_DSP_DMA_TRANSFER_Q0 */
	0x80022214, /* RAAGA_DSP_INTH_HOST_MASK_CLEAR */
	0x80022210, /* RAAGA_DSP_INTH_HOST_MASK_SET */
	0x80022204, /* RAAGA_DSP_INTH_HOST_SET */
	0x8002145c, /* RAAGA_DSP_DMA_TOKEN_ID_CLR_Q0 */
	0x80023010, /* Base Address for FIFO 0 */
	0x80023014, /* End Address for FIFO 0 */
	0x80023018, /* Write Pointer for FIFO 0 */
	0x8002301c, /* Read Pointer for FIFO 0 */
	0x80023020, /* Base Address for FIFO 1 */
	0x800215a8, /* RAAGA_DSP_DMA_SRC_ADDR_VQ4 */
	0x800215ac, /* RAAGA_DSP_DMA_DEST_ADDR_VQ4 */
	0x800215b0, /* RAAGA_DSP_DMA_TRANSFER_VQ4 */
	0x800215b4, /* RAAGA_DSP_DMA_PROGRESS_VQ4 */
	0x800215bc, /* RAAGA_DSP_DMA_TOKEN_ID_CLR_VQ4 */
	0x800215a0, /* RAAGA_DSP_DMA_MAX_SCB_BURST_SIZE_VQ4 */
	0x800215a4, /* RAAGA_DSP_DMA_SCB_GEN_CMD_TYPE_VQ4 */
	0x800215c4, /* RAAGA_DSP_DMA_DMA_ADDR1_VQ4 */
	0x800215c8, /* RAAGA_DSP_DMA_DMA_ADDR2_VQ4 */
	0x800215cc, /* RAAGA_DSP_DMA_VIDEO_PATCH_PARAM1_VQ4 */
	0x800215d0, /* RAAGA_DSP_DMA_VIDEO_PATCH_PARAM2_VQ4 */
	0x800215f8, /* RAAGA_DSP_DMA_SRC_ADDR_VQ5 */
	0x80021564, /* RAAGA_DSP_DMA_DRAM_VIDEO_NMBY0 */
	0x80021544, /* RAAGA_DSP_DMA_DRAM_VIDEO_BASE_ADDR0 */
	0x80021540, /* RAAGA_DSP_DMA_SCB_IF_CONFIG */
	0xf04e612c, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_REGISTER */
	0x20000000, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_MASK */
	0x00000019, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_SHIFT */
	0xf04e6124, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_REGISTER */
	0x00000002, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_MASK */
	0x00000001, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_SHIFT */
	0x00000007, /* NUM_MEM_PROTECT_REGIONS */
	0x00000000, /* MEM_PROTECT_REGION0_START */
	0x3fffffff, /* MEM_PROTECT_REGION0_END */
	0x40020000, /* MEM_PROTECT_REGION1_START */
	0x4fffffff, /* MEM_PROTECT_REGION1_END */
	0x50080000, /* MEM_PROTECT_REGION2_START */
	0x7fffffff, /* MEM_PROTECT_REGION2_END */
	0x82000000, /* MEM_PROTECT_REGION3_START */
	0xf0a07fff, /* MEM_PROTECT_REGION3_END */
	0xf0a0ab00, /* MEM_PROTECT_REGION4_START */
	0xf0a3ffff, /* MEM_PROTECT_REGION4_END */
#if (BCHP_VER == BCHP_VER_A0)   /* A0 */
	0xf0a4d6f8, /* MEM_PROTECT_REGION5_START */
	0xf0bfffff, /* MEM_PROTECT_REGION5_END */
#elif (BCHP_VER == BCHP_VER_B0) /* B0 */
	0xf0a4e17c, /* MEM_PROTECT_REGION5_START */
	0xf0bfffff, /* MEM_PROTECT_REGION5_END */
#else /* C0 */
	0xf0a60000, /* MEM_PROTECT_REGION5_START */
	0xf0bfffff, /* MEM_PROTECT_REGION5_END */
#endif
	0xf0d00000, /* MEM_PROTECT_REGION6_START */
	0xffffffff, /* MEM_PROTECT_REGION6_END */
	0x00000000, /* MEM_PROTECT_REGION7_START */
	0x00000000, /* MEM_PROTECT_REGION7_END */
	0x00000000, /* MEM_PROTECT_REGION8_START */
	0x00000000, /* MEM_PROTECT_REGION8_END */
	0x00000000, /* MEM_PROTECT_REGION9_START */
	0x00000000, /* MEM_PROTECT_REGION9_END */
	0xf0404008, /* SUN_TOP_CTRL_BSP_FEATURE_TABLE_ADDR */
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
