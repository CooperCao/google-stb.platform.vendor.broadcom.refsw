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
 *****************************************************************************/

#include "bchp.h"
const uint32_t BDSP_IMG_system_rdbvars_array1[] = {
        0x00030000, /* IMEM_SIZE */
        0x00020000, /* DMEM_SIZE */
        0x00000049, /* UART_BAUD_RATE_DIV_FACTOR */
        0x20E6DA00, /* DSP_FREQUENCY */
        0x80021080, /* DSP Subsystem UART Status */
        0x80021084, /* DSP Subsystem UART Receive Data */
        0x80021088, /* DSP Subsystem UART Transmit Data */
        0x8002108C, /* DSP Subsystem UART Control */
        0x80021000, /* TSM Timer Control Register */
        0x80021004, /* Time of the TSM Timer */
        0x80023000, /* FIFO 0 BASEADDRESS */
        0x80023004, /* FIFO ID Containing HOST2DSP Command */
        0x80021140, /* SW SET CLEAR Status Register (Register 0) */
        0x80021144, /* SW SET CLEAR Set Register (Register 0) */
        0x80021148, /* SW SET CLEAR Clear Register (Register 0) */
        0x8002114C, /* SW SET CLEAR Status Register (Register 1) */
        0x80021150, /* SW SET CLEAR Set Register (Register 1) */
        0x80021154, /* SW SET CLEAR Clear Register (Register 1) */
        0x80022404, /* Host Interrupt Set Register */
        0x80021054, /* Watchdog Timer Control Register */
        0x80021058, /* Time of the Watchdog Timer */
        0x80034000, /* MDMEM RAM */
        0x800233c0, /* Base Address for FIFO DRAMLOGS */
        0x800233c4, /* End Address for FIFO DRAMLOGS */
        0x800233c8, /* Write Pointer for FIFO DRAMLOGS */
        0x800233cc, /* Read Pointer for FIFO DRAMLOGS */
        0x80038000, /* VOM RAM */
        0x80020000, /* Audio DSP System Revision Register */
        0x80021400, /* Priority of DMA Queues */
        0x80021120, /* Mailbox Register 0 */
        0x80021124, /* Mailbox Register 1 */
        0x80021128, /* Mailbox Register 2 */
        0x8002112C, /* Mailbox Register 3 */
        0x80021130, /* Mailbox Register 4 */
        0x80021134, /* Mailbox Register 5 */
        0x80021138, /* Mailbox Register 6 */
        0x8002113C, /* Mailbox Register 7 */
        0x80023500, /* Reserved Register 0 */
        0x80021448, /* DMA Source Address Register for DMA Command Queue-0 */
        0x80021488, /* DMA Source Address Register for DMA command Queue-1 */
        0x8002144C, /* DMA destination address register for DMA Command Queue-0 */
        0x80021444, /* DMA SCB command type for Video Block Access and Video Raster access commands. */
        0x80021450, /* DMA transfer enable register for DMA-Queue-0 */
        0x80022214, /* Host Interrupt Mask Clear Register */
        0x80022210, /* Host Interrupt Mask Set Register */
        0x80022204, /* Host Interrupt Set Register */
        0x8002145C, /* DMA-Token-ID clear for DMA-Queue-0 */
        0x80023010, /* Base Address for FIFO 0 */
        0x80023014, /* End Address for FIFO 0 */
        0x80023018, /* Write Pointer for FIFO 0 */
        0x8002301c, /* Read Pointer for FIFO 0 */
        0x80023020, /* Base Address for FIFO 1 */
        0X800215A8, /* RAAGA_DSP_DMA_SRC_ADDR_VQ4 */
        0X800215AC, /* RAAGA_DSP_DMA_DEST_ADDR_VQ4 */
        0X800215B0, /* RAAGA_DSP_DMA_TRANSFER_VQ4 */
        0X800215B4, /* RAAGA_DSP_DMA_PROGRESS_VQ4 */
        0X800215BC, /* RAAGA_DSP_DMA_TOKEN_ID_CLR_VQ4 */
        0X800215A0, /* RAAGA_DSP_DMA_MAX_SCB_BURST_SIZE_VQ4 */
        0X800215A4, /* RAAGA_DSP_DMA_SCB_GEN_CMD_TYPE_VQ4 */
        0X800215C4, /* RAAGA_DSP_DMA_DMA_ADDR1_VQ4 */
        0X800215C8, /* RAAGA_DSP_DMA_DMA_ADDR2_VQ4 */
        0X800215CC, /* RAAGA_DSP_DMA_VIDEO_PATCH_PARAM1_VQ4 */
        0X800215D0, /* RAAGA_DSP_DMA_VIDEO_PATCH_PARAM2_VQ4 */
        0X800215F8, /* RAAGA_DSP_DMA_SRC_ADDR_VQ5 */
        0X80021564, /* RAAGA_DSP_DMA_DRAM_VIDEO_NMBY0 */
        0X80021544, /* RAAGA_DSP_DMA_DRAM_VIDEO_BASE_ADDR0 */
        0X80021540, /* RAAGA_DSP_DMA_SCB_IF_CONFIG */
        0x10374124, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_REGISTER */
        0x00080000, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_MASK */
        0x00000013, /* RAAGA_DSP_DOLBY_LICENSE_OTP0_SHIFT */
        0x10374124, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_REGISTER */
        0x00040000, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_MASK */
        0x00000012, /* RAAGA_DSP_DOLBY_LICENSE_OTP1_SHIFT */
        0x00000007, /* NUM_MEM_PROTECT_REGIONS */
        0x00000000, /* MEM_PROTECT_REGION0_START */
        0x107fffff, /* MEM_PROTECT_REGION0_END */
        0x10900000, /* MEM_PROTECT_REGION1_START */
        0x10a0bfff, /* MEM_PROTECT_REGION1_END */
        0x10a0d900, /* MEM_PROTECT_REGION2_START */
        0x10a1ffff, /* MEM_PROTECT_REGION2_END */
        0x10a30000, /* MEM_PROTECT_REGION3_START */
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
