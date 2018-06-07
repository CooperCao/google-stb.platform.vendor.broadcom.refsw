/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/


#ifndef BDSP_RAAGA_INT_PRIV_H_
#define BDSP_RAAGA_INT_PRIV_H_
#include "bdsp_raaga_types.h"
#include "bdsp_raaga_priv.h"


BERR_Code BDSP_Raaga_P_AckInstall(
    void *pDeviceHandle
);
BERR_Code BDSP_Raaga_P_AckUninstall(
    void *pDeviceHandle
);

BERR_Code BDSP_Raaga_P_InterruptInstall (
    void    *pTaskHandle        /* [in] Raptor Channel handle */
);

BERR_Code BDSP_Raaga_P_InterruptUninstall (
    void    *pTaskHandle        /* [in] Raptor Channel handle */
);


BERR_Code BDSP_Raaga_P_ContextInterruptInstall (
    void    *pContextHandle
);

BERR_Code BDSP_Raaga_P_ContextInterruptUninstall (
    void    *pContextHandle
);

void BDSP_Raaga_P_GetAudioInterruptHandlers_isr(
    void *pTaskHandle,
    BDSP_AudioInterruptHandlers *pHandlers);

BERR_Code BDSP_Raaga_P_SetAudioInterruptHandlers_isr(
    void *pTaskHandle,
    const BDSP_AudioInterruptHandlers *pHandlers);

BERR_Code BDSP_Raaga_P_SetInterruptHandlers(
    void *pContextHandle,
    const BDSP_ContextInterruptHandlers *pInterrupts);

BERR_Code BDSP_Raaga_P_ProcessWatchdogInterrupt(
    void *pContextHandle);

void BDSP_Raaga_P_GetInterruptHandlers(
    void *pContextHandle,
    BDSP_ContextInterruptHandlers *pInterrupts);

void BDSP_Raaga_P_RestoreDspWatchdogTimer (
                       void    *pDeviceHandle,
                       uint32_t dspIndex);

void BDSP_Raaga_P_RestoreWatchdogTimer (
                       void    *pDeviceHandle);

void BDSP_Raaga_P_EnableDspWatchdogTimer (
                       void    *pDeviceHandle,
                       uint32_t dspIndex,
                       bool     bEnable
                       );

void BDSP_Raaga_P_EnableWatchdogTimer (
                       void    *pDeviceHandle,
                       bool     bEnable
                       );

void BDSP_Raaga_P_EnableAndSaveStateWatchdogTimer (
                       void    *pDeviceHandle,
                       bool     bEnable
                       );

#endif
