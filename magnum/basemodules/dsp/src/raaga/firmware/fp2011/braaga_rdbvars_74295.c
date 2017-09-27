/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
        0x00030000, /* Imem Size */
        0x00020000, /* Dmem Size */
        0x00000049, /* Uart baud rate div factor */
        0x20E6DA00, /* Firepath frequency in Hz */
        0x80021080, /* DSP Subsystem UART Status */
        0x80021084, /* DSP Subsystem UART Receive Data */
        0x80021088, /* DSP Subsystem UART Transmit Data */
        0x8002108c, /* DSP Subsystem UART Control */
        0x80021000, /* TSM Timer Control Register */
        0x80021004, /* Time of the TSM Timer */
        0x80023000, /* FIFO 0 BASEADDRESS */
        0x80023004, /* FIFO ID Containing HOST2DSP Command */
        0x80021140, /* SW SET CLEAR Status Register (Register 0) */
        0x80021144, /* SW SET CLEAR Set Register (Register 0) */
        0x80021148, /* SW SET CLEAR Clear Register (Register 0) */
        0x8002114c, /* SW SET CLEAR Status Register (Register 1) */
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
        0x8002112c, /* Mailbox Register 3 */
        0x80021130, /* Mailbox Register 4 */
        0x80021134, /* Mailbox Register 5 */
        0x80021138, /* Mailbox Register 6 */
        0x8002113c, /* Mailbox Register 7 */
        0x80023500, /* Reserved Register 0 */
        0x80021448, /* DMA Source Address Register for DMA Command Queue-0 */
        0x80021488, /* DMA Source Address Register for DMA command Queue-1 */
        0x8002144c, /* DMA destination address register for DMA Command Queue-0 */
        0x80021444, /* DMA SCB command type for Video Block Access and Video Raster access commands. */
        0x80021450, /* DMA transfer enable register for DMA-Queue-0 */
        0x80022214, /* Host Interrupt Mask Clear Register */
        0x80022210, /* Host Interrupt Mask Set Register */
        0x80022204, /* Host Interrupt Set Register */
        0x8002145c, /* DMA-Token-ID clear for DMA-Queue-0 */
        0x80023010, /* Base Address for FIFO 0 */
        0x80023014, /* End Address for FIFO 0 */
        0x80023018, /* Write Pointer for FIFO 0 */
        0x8002301c, /* Read Pointer for FIFO 0 */
        0x80023020, /* Base Address for FIFO 1 */
        0x800215a8, /* DMA Source Address Register for Video Queue-4 for non-Pixel patch operations */
        0x800215ac, /* DMA destination address register for Queue-4 for non-Pixel patch operations */
        0x800215b0, /* DMA transfer enable register for DMA-Video Queue-4 */
        0x800215b4, /* DMA transfer progress register for DMA-Queue-0 */
        0x800215bc, /* DMA-Token-ID clear for DMA-VQ4 */
        0x800215a0, /* DMA maximum SCB command burst size for DMA-Video Queue-4 */
        0x800215a4, /* DMA SCB command type for SCB_VIDEO_ACCESS command for Video Queue # 4. */
        0x800215c4, /* DMA Address1 Register for Video Queue-4 */
        0x800215c8, /* DMA Address2 Register for Video Queue-4 */
        0x800215cc, /* Video Patch offset register for Video Queue 4 */
        0x800215d0, /* Video Patch operation parameters */
        0x800215f8, /* DMA Source Address Register for Video Queue-5 for non-Pixel Patch operations */
        0x80021564, /* NMBY for DRAM Video Base Address 0 */
        0x80021544, /* DRAM Video Base Address 0 */
        0x80021540, /* Configuration register SCB0/1 MS bits programming and stripe width selection */
        0x10374138,
        0x00000002,
        0x00000001,
        0x10374130,
        0x00000001,
        0x00000000,
        0x00000007, /* NUM_MEM_PROTECT_REGIONS */
        0x00000000, /* MEM_PROTECT_REGION0_START */
        0x1020bfff, /* MEM_PROTECT_REGION0_END */
        0x1020d900, /* MEM_PROTECT_REGION1_START */
        0x1021ffff, /* MEM_PROTECT_REGION1_END */
        0x10228db0, /* MEM_PROTECT_REGION2_START */
        0x107fffff, /* MEM_PROTECT_REGION2_END */
        0x10900000, /* MEM_PROTECT_REGION3_START */
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
        0x10404008,
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
        0x00000000, /* Not Used */
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
