/*******************************************************************************
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
 ******************************************************************************/

#include "bchp.h"

const uint32_t BDSP_ARM_IMG_system_rdbvars_array1[] = {
  4,/*BDSP_ARM_P_eRdbVarIndices_DSP_PADDR_WIDTH_IN_BYTES*/
  1,/*BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_HOST2DSPCMD_FIFO_ID*/
  2,/*BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_HOST2DSPRESPONSE_FIFO_ID*/
  3,/*BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_BASE_ADDR*/
  4,/*BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_END_ADDR*/
  5,/*BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_WRITE_ADDR*/
  6,/*BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_READ_ADDR*/
  7,/*BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_1_BASE_ADDR*/
  0,/*eUART_BAUD_RATE_DIV_FACTOR*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_PERI_DBG_CTRL_UART_STATUS*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_PERI_DBG_CTRL_UART_RCV_DATA*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_PERI_DBG_CTRL_UART_XMIT_DATA*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_PERI_DBG_CTRL_UART_CTRL*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_TIMERS_TSM_TIMER*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_TIMERS_TSM_TIMER_VALUE*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_STATUS_0*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_SET_0*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_CLEAR_0*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_STATUS_1*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_SET_1*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_CLEAR_1*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_FW_INTH_HOST_SET*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_DOLBY_LICENSE_OTP0_REGISTER*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_DOLBY_LICENSE_OTP0_MASK*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_DOLBY_LICENSE_OTP0_SHIFT*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_DOLBY_LICENSE_OTP1_REGISTER*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_DOLBY_LICENSE_OTP1_MASK*/
  0,/*BDSP_ARM_P_eRdbVarIndices_DSP_DOLBY_LICENSE_OTP1_SHIFT*/
  0,/*eSUN_TOP_CTRL_BSP_FEATURE_TABLE_ADDR*/
  0xf0c80000,/*BDSP_ARM_P_eRdbVarIndices_DSP_RAAGA_REG_REGION_BASE_ADDR*/
  0x00040090,/*BDSP_ARM_P_eRdbVarIndices_DSP_RAAGA_REG_REGION_SIZE*/
  0xf0a00000,/*BDSP_ARM_P_eRdbVarIndices_DSP_XPT_REG_REGION_BASE_ADDR*/
  0x00080000,/*BDSP_ARM_P_eRdbVarIndices_DSP_XPT_REG_REGION_SIZE*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
  0,/*Not used*/
};
const uint32_t BDSP_ARM_IMG_system_rdbvars_header [2] = {sizeof(BDSP_ARM_IMG_system_rdbvars_array1), 1};
const void * const BDSP_ARM_IMG_system_rdbvars [2] = {BDSP_ARM_IMG_system_rdbvars_header, BDSP_ARM_IMG_system_rdbvars_array1};