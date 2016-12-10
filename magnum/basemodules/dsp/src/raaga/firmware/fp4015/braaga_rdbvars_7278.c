/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * PLEASE DONOT MODIFY THIS FILE MANUALLY. This is an auto-generated file
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
    0x80021000, /* RAAGA_DSP_TIMERS_TSM_TIMER */
    0x80021004, /* RAAGA_DSP_TIMERS_TSM_TIMER_VALUE */
    0x80023000, /* RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO0_BASEADDR */
    0x80023004, /* RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO_ID */
    0x80021344, /* RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_0 */
    0x80021348, /* RAAGA_DSP_PERI_SW_MSG_BITS_SET_0 */
    0x8002134c, /* RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_0 */
    0x80021350, /* RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_1 */
    0x80021354, /* RAAGA_DSP_PERI_SW_MSG_BITS_SET_1 */
    0x80021358, /* RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_1 */
    0x80022904, /* RAAGA_DSP_FW_INTH_HOST_SET */
    0x80021054, /* RAAGA_DSP_TIMERS_WATCHDOG_TIMER */
    0x80021058, /* RAAGA_DSP_TIMERS_WATCHDOG_TIMER_VALUE */
    0x00000000, /* RAAGA_DSP_MEM_SUBSYSTEM_MDMEMRAM0 */
    0x800233c0, /* RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_BASE_ADDR */
    0x800233c4, /* RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_END_ADDR */
    0x800233c8, /* RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_WRITE_ADDR */
    0x800233cc, /* RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_READ_ADDR */
    0x00000000, /* RAAGA_DSP_MEM_SUBSYSTEM_VOMRAM0 */
    0x80020000, /* RAAGA_DSP_MISC_REVISION */
    0x80021400, /* RAAGA_DSP_DMA_QUEUE_PRIORITY */
    0x80021324, /* RAAGA_DSP_PERI_SW_MAILBOX0 */
    0x80021328, /* RAAGA_DSP_PERI_SW_MAILBOX1 */
    0x8002132c, /* RAAGA_DSP_PERI_SW_MAILBOX2 */
    0x80021330, /* RAAGA_DSP_PERI_SW_MAILBOX3 */
    0x80021334, /* RAAGA_DSP_PERI_SW_MAILBOX4 */
    0x80021338, /* RAAGA_DSP_PERI_SW_MAILBOX5 */
    0x8002133c, /* RAAGA_DSP_PERI_SW_MAILBOX6 */
    0x80021340, /* RAAGA_DSP_PERI_SW_MAILBOX7 */
    0x80023500, /* RAAGA_DSP_FW_CFG_SW_DEBUG_SPARE0 */
    0x80021448, /* RAAGA_DSP_DMA_SRC_ADDR_Q0 */
    0x80021488, /* RAAGA_DSP_DMA_SRC_ADDR_Q1 */
    0x80021450, /* RAAGA_DSP_DMA_DEST_ADDR_Q0 */
    0x80021444, /* RAAGA_DSP_DMA_SCB_GEN_CMD_TYPE_Q0 */
    0x80021458, /* RAAGA_DSP_DMA_TRANSFER_Q0 */
    0x80022114, /* RAAGA_DSP_INTH_HOST_MASK_CLEAR */
    0x80022110, /* RAAGA_DSP_INTH_HOST_MASK_SET */
    0x80022104, /* RAAGA_DSP_INTH_HOST_SET */
    0x80021464, /* RAAGA_DSP_DMA_TOKEN_ID_CLR_Q0 */
    0x80023010, /* RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR */
    0x80023014, /* RAAGA_DSP_FW_CFG_FIFO_0_END_ADDR */
    0x80023018, /* RAAGA_DSP_FW_CFG_FIFO_0_WRITE_ADDR */
    0x8002301c, /* RAAGA_DSP_FW_CFG_FIFO_0_READ_ADDR */
    0x80023020, /* RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR */
    0x800215c8, /* RAAGA_DSP_DMA_SRC_ADDR_VQ4 */
    0x800215d0, /* RAAGA_DSP_DMA_DEST_ADDR_VQ4 */
    0x800215d8, /* RAAGA_DSP_DMA_TRANSFER_VQ4 */
    0x800215dc, /* RAAGA_DSP_DMA_PROGRESS_VQ4 */
    0x800215e4, /* RAAGA_DSP_DMA_TOKEN_ID_CLR_VQ4 */
    0x800215c0, /* RAAGA_DSP_DMA_MAX_SCB_BURST_SIZE_VQ4 */
    0x800215c4, /* RAAGA_DSP_DMA_SCB_GEN_CMD_TYPE_VQ4 */
    0x090215f0, /* RAAGA_DSP_DMA_DMA_ADDR1_VQ4 */
    0x090215f8, /* RAAGA_DSP_DMA_DMA_ADDR2_VQ4 */
    0x80021600, /* RAAGA_DSP_DMA_VIDEO_PATCH_PARAM1_VQ4 */
    0x80021604, /* RAAGA_DSP_DMA_VIDEO_PATCH_PARAM2_VQ4 */
    0x80021628, /* RAAGA_DSP_DMA_SRC_ADDR_VQ5 */
    0x80021588, /* RAAGA_DSP_DMA_DRAM_VIDEO_NMBY0 */
    0x80021548, /* RAAGA_DSP_DMA_DRAM_VIDEO_BASE_ADDR0 */
    0x80021540, /* RAAGA_DSP_DMA_SCB_IF_CONFIG */
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
