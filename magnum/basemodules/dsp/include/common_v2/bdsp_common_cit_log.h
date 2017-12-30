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
#ifndef BDSP_COMMON_CIT_LOG_H_
#define BDSP_COMMON_CIT_LOG_H_

#include "bdsp_common_priv_include.h"

#define BDSP_MAX_CHAR_LENGTH  80

void BDSP_P_Analyse_CIT_PortDetails(
	BDSP_AF_P_sIoPort *pPortDetails,
	BDSP_P_FwBuffer   *pDescriptorMemory
);
void BDSP_P_Analyse_CIT_TSMConfig_isr(
	BDSP_MMA_Memory Memory
);
void BDSP_P_Analyse_CIT_GateOpenConfig(
	BDSP_MMA_Memory Memory
);
void BDSP_P_Analyse_CIT_StcTriggerConfig(
	BDSP_MMA_Memory Memory
);
void BDSP_P_Analyse_CIT_SchedulingConfig(
	BDSP_MMA_Memory Memory
);
void BDSP_P_Analyse_CIT_PPMConfig(
	BDSP_AF_P_sHW_PPM_CONFIG    *psPPMConfig
);
void BDSP_P_Analyse_CIT_DataAccess(
	BDSP_AF_P_Port_sDataAccessAttributes *pDataAccessAttributes
);
void BDSP_P_Analyse_CIT_BufferDetails(
	BDSP_P_FwBuffer   *pDescriptorMemory,
	dramaddr_t 		   offset,
	unsigned           index
);

extern const char Algorithm2Name[BDSP_Algorithm_eMax][BDSP_MAX_CHAR_LENGTH];
extern const char PortType[BDSP_AF_P_PortType_eLast][BDSP_MAX_CHAR_LENGTH];
extern const char DataType[BDSP_DataType_eMax][BDSP_MAX_CHAR_LENGTH];
extern const char DistinctOutputType[BDSP_AF_P_DistinctOpType_eMax][BDSP_MAX_CHAR_LENGTH];
extern const char BufferType[BDSP_AF_P_BufferType_eLast][BDSP_MAX_CHAR_LENGTH];
extern const char DataAccessType[BDSP_AF_P_Port_eLast][BDSP_MAX_CHAR_LENGTH];
extern const char BaseRateMultiplier[BDSP_AF_P_FmmDstFsRate_eMax+1][BDSP_MAX_CHAR_LENGTH];
extern const char FMMContentType [BDSP_AF_P_FmmContentType_eMax+1][BDSP_MAX_CHAR_LENGTH];
extern const char GlobalTimeBase [2][BDSP_MAX_CHAR_LENGTH];
extern const char DelayMode[BDSP_AudioTaskDelayMode_eMax+1][BDSP_MAX_CHAR_LENGTH];
extern const char DisableEnable[2][BDSP_MAX_CHAR_LENGTH];
extern const char TrueFalse[2][BDSP_MAX_CHAR_LENGTH];
extern const char ContextType[BDSP_ContextType_eMax+1][BDSP_MAX_CHAR_LENGTH];
extern const char FirmwareSchedulingType[BDSP_P_SchedulingMode_eLast][BDSP_MAX_CHAR_LENGTH];
extern const char SchedulingMode[BDSP_TaskSchedulingMode_eMax+1][BDSP_MAX_CHAR_LENGTH];
extern const char SchedulingType[BDSP_TaskRealtimeMode_eMax+1][BDSP_MAX_CHAR_LENGTH];
extern const char TaskType[BDSP_P_TaskType_eLast][BDSP_MAX_CHAR_LENGTH];
#endif /*BDSP_COMMON_CIT_LOG_H_*/
