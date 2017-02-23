/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bchp.h"
const uint32_t BDSP_IMG_system_rdbvars_array1[] = {
    0x00000000, /* IMEM_SIZE */
    0x00000000, /* DMEM_SIZE */
    0x0000001C, /* UART_BAUD_RATE_DIV_FACTOR */
    0x26BE3680, /* DSP_FREQUENCY */
    0x09021080, /* RAAGA_DSP_PERI_DBG_CTRL_UART_STATUS */
    0x09021084, /* RAAGA_DSP_PERI_DBG_CTRL_UART_RCV_DATA */
    0x09021088, /* RAAGA_DSP_PERI_DBG_CTRL_UART_XMIT_DATA */
    0x0902108c, /* RAAGA_DSP_PERI_DBG_CTRL_UART_CTRL */
    0x09021000, /* RAAGA_DSP_TIMERS_TSM_TIMER */
    0x09021004, /* RAAGA_DSP_TIMERS_TSM_TIMER_VALUE */
    0x09023000, /* RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO0_BASEADDR */
    0x09023008, /* RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO_ID */
    0x09021344, /* RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_0 */
    0x09021348, /* RAAGA_DSP_PERI_SW_MSG_BITS_SET_0 */
    0x0902134c, /* RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_0 */
    0x09021350, /* RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_1 */
    0x09021354, /* RAAGA_DSP_PERI_SW_MSG_BITS_SET_1 */
    0x09021358, /* RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_1 */
    0x09022904, /* RAAGA_DSP_FW_INTH_HOST_SET */
    0x09021054, /* RAAGA_DSP_TIMERS_WATCHDOG_TIMER */
    0x09021058, /* RAAGA_DSP_TIMERS_WATCHDOG_TIMER_VALUE */
    0x00000000, /* RAAGA_DSP_MEM_SUBSYSTEM_MDMEMRAM0 */
    0x09023780, /* RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_BASE_ADDR */
    0x09023788, /* RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_END_ADDR */
    0x09023790, /* RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_WRITE_ADDR */
    0x09023798, /* RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_READ_ADDR */
    0x00000000, /* RAAGA_DSP_MEM_SUBSYSTEM_VOMRAM0 */
    0x09020000, /* RAAGA_DSP_MISC_REVISION */
    0x09021400, /* RAAGA_DSP_DMA_QUEUE_PRIORITY */
    0x09021324, /* RAAGA_DSP_PERI_SW_MAILBOX0 */
    0x09021328, /* RAAGA_DSP_PERI_SW_MAILBOX1 */
    0x0902132c, /* RAAGA_DSP_PERI_SW_MAILBOX2 */
    0x09021330, /* RAAGA_DSP_PERI_SW_MAILBOX3 */
    0x09021334, /* RAAGA_DSP_PERI_SW_MAILBOX4 */
    0x09021338, /* RAAGA_DSP_PERI_SW_MAILBOX5 */
    0x0902133c, /* RAAGA_DSP_PERI_SW_MAILBOX6 */
    0x09021340, /* RAAGA_DSP_PERI_SW_MAILBOX7 */
    0x09023930, /* RAAGA_DSP_FW_CFG_SW_DEBUG_SPARE0 */
    0x09021448, /* RAAGA_DSP_DMA_SRC_ADDR_Q0 */
    0x09021488, /* RAAGA_DSP_DMA_SRC_ADDR_Q1 */
    0x09021450, /* RAAGA_DSP_DMA_DEST_ADDR_Q0 */
    0x09021444, /* RAAGA_DSP_DMA_SCB_GEN_CMD_TYPE_Q0 */
    0x09021458, /* RAAGA_DSP_DMA_TRANSFER_Q0 */
    0x09022114, /* RAAGA_DSP_INTH_HOST_MASK_CLEAR */
    0x09022110, /* RAAGA_DSP_INTH_HOST_MASK_SET */
    0x09022104, /* RAAGA_DSP_INTH_HOST_SET */
    0x09021464, /* RAAGA_DSP_DMA_TOKEN_ID_CLR_Q0 */
    0x09023020, /* RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR */
    0x09023028, /* RAAGA_DSP_FW_CFG_FIFO_0_END_ADDR */
    0x09023030, /* RAAGA_DSP_FW_CFG_FIFO_0_WRITE_ADDR */
    0x09023038, /* RAAGA_DSP_FW_CFG_FIFO_0_READ_ADDR */
    0x09023040, /* RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR */
    0x090215c8, /* RAAGA_DSP_DMA_SRC_ADDR_VQ4 */
    0x090215d0, /* RAAGA_DSP_DMA_DEST_ADDR_VQ4 */
    0x090215d8, /* RAAGA_DSP_DMA_TRANSFER_VQ4 */
    0x090215dc, /* RAAGA_DSP_DMA_PROGRESS_VQ4 */
    0x090215e4, /* RAAGA_DSP_DMA_TOKEN_ID_CLR_VQ4 */
    0x090215c0, /* RAAGA_DSP_DMA_MAX_SCB_BURST_SIZE_VQ4 */
    0x090215c4, /* RAAGA_DSP_DMA_SCB_GEN_CMD_TYPE_VQ4 */
    0x090215f0, /* RAAGA_DSP_DMA_DMA_ADDR1_VQ4 */
    0x090215f8, /* RAAGA_DSP_DMA_DMA_ADDR2_VQ4 */
    0x09021600, /* RAAGA_DSP_DMA_VIDEO_PATCH_PARAM1_VQ4 */
    0x09021604, /* RAAGA_DSP_DMA_VIDEO_PATCH_PARAM2_VQ4 */
    0x09021628, /* RAAGA_DSP_DMA_SRC_ADDR_VQ5 */
    0x09021588, /* RAAGA_DSP_DMA_DRAM_VIDEO_NMBY0 */
    0x09021548, /* RAAGA_DSP_DMA_DRAM_VIDEO_BASE_ADDR0 */
    0x09021540, /* RAAGA_DSP_DMA_SCB_IF_CONFIG */
    0x0845712c, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_REGISTER */
    0x02000000, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_MASK */
    0x00000019, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_SHIFT */
    0x0845712c, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_REGISTER */
    0x01000000, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_MASK */
    0x00000018, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_SHIFT */
    0x00000000, /* NUM_MEM_PROTECT_REGIONS */
    0x00000000, /* MEM_PROTECT_REGION0_START */
    0x00000000, /* MEM_PROTECT_REGION0_END */
    0x00000000, /* MEM_PROTECT_REGION1_START */
    0x00000000, /* MEM_PROTECT_REGION1_END */
    0x00000000, /* MEM_PROTECT_REGION2_START */
    0x00000000, /* MEM_PROTECT_REGION2_END */
    0x00000000, /* MEM_PROTECT_REGION3_START */
    0x00000000, /* MEM_PROTECT_REGION3_END */
    0x00000000, /* MEM_PROTECT_REGION4_START */
    0x00000000, /* MEM_PROTECT_REGION4_END */
    0x00000000, /* MEM_PROTECT_REGION5_START */
    0x00000000, /* MEM_PROTECT_REGION5_END */
    0x00000000, /* MEM_PROTECT_REGION6_START */
    0x00000000, /* MEM_PROTECT_REGION6_END */
    0x00000000, /* MEM_PROTECT_REGION7_START */
    0x00000000, /* MEM_PROTECT_REGION7_END */
    0x00000000, /* MEM_PROTECT_REGION8_START */
    0x00000000, /* MEM_PROTECT_REGION8_END */
    0x00000000, /* MEM_PROTECT_REGION9_START */
    0x00000000, /* MEM_PROTECT_REGION9_END */
    0x08404008, /* SUN_TOP_CTRL_BSP_FEATURE_TABLE_ADDR */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
    0x00000000, /* Not Used */
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
