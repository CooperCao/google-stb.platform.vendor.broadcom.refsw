/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
*   
*  Except as expressly set forth in the Authorized License,
*   
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*   
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
*  USE OR PERFORMANCE OF THE SOFTWARE.
*  
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
*  ANY LIMITED REMEDY.
* 
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: DvbCi
*    Specific APIs related DVB Common Interface
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef NEXUS_DVB_CI_MODULE_H__
#define NEXUS_DVB_CI_MODULE_H__

#include "nexus_base.h"
#include "nexus_dvb_ci_thunks.h"
#include "nexus_dvb_ci.h"
#include "nexus_dvb_ci_init.h"
#include "nexus_types.h"
#include "nexus_gpio.h"
#include "priv/nexus_core.h"
#ifdef NEXUS_DVB_CI_EXTENSION
#include "nexus_dvb_ci_extension.h"
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME dvb_ci
#define NEXUS_MODULE_SELF g_dvbCiModule

extern NEXUS_ModuleHandle g_dvbCiModule;

BDBG_OBJECT_ID_DECLARE(NEXUS_DvbCi);

/* States of the physical interface */
typedef enum NEXUS_DvbCiState
{
    NEXUS_DvbCiState_eInit,
    NEXUS_DvbCiState_eNoCard,
    NEXUS_DvbCiState_ePcmciaReset,
    NEXUS_DvbCiState_ePcmciaResetTimeout,
    NEXUS_DvbCiState_eReadCis,
    NEXUS_DvbCiState_eInvalidCis,
    NEXUS_DvbCiState_eWroteCor,
    NEXUS_DvbCiState_eNegotiated,
    NEXUS_DvbCiState_eTransferError,
    NEXUS_DvbCiState_eMax
} NEXUS_DvbCiState;

/* Object structure */
typedef struct NEXUS_DvbCi
{
    BDBG_OBJECT(NEXUS_DvbCi)
    NEXUS_DvbCiOpenSettings openSettings;
    NEXUS_DvbCiSettings settings;
    NEXUS_DvbCiError lastError;     /* The current error state of the card */
    uint16_t negotiatedBufferSize;  /* The negotiated buffer size between the host and card */
    bool validCis;
    bool interruptsSupported;
    bool pcmciaMode;
    NEXUS_DvbCiState currentState;
    uint8_t *pBaseAddress;
    uint8_t *pIOBaseAddress;
    BKNI_EventHandle cdEvent;
    NEXUS_EventCallbackHandle cdEventCallback;
    BKNI_EventHandle readyEvent;
    NEXUS_TimerHandle readyTimer;
    NEXUS_EventCallbackHandle readyEventCallback;
    NEXUS_TimerHandle pollTimer;
    NEXUS_TaskCallbackHandle errorCallback, cardDetectCallback, ireqCallback;
    char manufacturerName[NEXUS_DVB_CI_MANUFACTURER_NAME_MAX];
    char productName[NEXUS_DVB_CI_PRODUCT_NAME_MAX];
    char additionalInfo[NEXUS_DVB_CI_ADDITIONAL_INFO_MAX];
    uint32_t corAddr;
    uint8_t corValue;
    bool vpp5v;
    bool daIE, frIE;
    bool fpgaTristate, fpgaPcmciaMode;
    NEXUS_IsrCallbackHandle cd1IrqCallback, cd2IrqCallback, ireqIrqCallback;
} NEXUS_DvbCi;

/* Read and parse the CIS from the card */
extern NEXUS_Error NEXUS_DvbCi_P_ReadCis(NEXUS_DvbCiHandle handle);

/* Required registers and bits */
#define NEXUS_DVB_CI_REG_DATA 0
#define NEXUS_DVB_CI_REG_COMMAND 1
#define NEXUS_DVB_CI_REG_STATUS 1
#define NEXUS_DVB_CI_REG_SIZE_LS 2
#define NEXUS_DVB_CI_REG_SIZE_MS 3

#define NEXUS_DVB_CI_STATUS_DA  0x80
#define NEXUS_DVB_CI_STATUS_FR  0x40
#define NEXUS_DVB_CI_STATUS_IIR 0x10
#define NEXUS_DVB_CI_STATUS_WE  0x02
#define NEXUS_DVB_CI_STATUS_RE  0x01

#define NEXUS_DVB_CI_COMMAND_DAIE 0x80
#define NEXUS_DVB_CI_COMMAND_FRIE 0x40
#define NEXUS_DVB_CI_COMMAND_RS   0x08
#define NEXUS_DVB_CI_COMMAND_SR   0x04
#define NEXUS_DVB_CI_COMMAND_SW   0x02
#define NEXUS_DVB_CI_COMMAND_HC   0x01

typedef enum NEXUS_DvbCiVpp
{
    NEXUS_DvbCiVpp_eTristate,
    NEXUS_DvbCiVpp_e3v,
    NEXUS_DvbCiVpp_e5v,
    NEXUS_DvbCiVpp_eMax
} NEXUS_DvbCiVpp;

typedef enum NEXUS_DvbCiVcc
{
    NEXUS_DvbCiVcc_eTristate,
    NEXUS_DvbCiVcc_e3v,
    NEXUS_DvbCiVcc_e5v,
    NEXUS_DvbCiVcc_eMax
} NEXUS_DvbCiVcc;

typedef enum NEXUS_DvbCiPinState
{
    NEXUS_DvbCiPinState_eLow,
    NEXUS_DvbCiPinState_eHigh,
    NEXUS_DvbCiPinState_eTristate,
    NEXUS_DvbCiPinState_eUnknown,
    NEXUS_DvbCiPinState_eMax
} NEXUS_DvbCiPinState;

/* Setup the transport stream pinmuxes for either DVB mode or PCMCIA mode -- Chip Specific */
extern NEXUS_Error NEXUS_DvbCi_P_SetupPinmuxes(NEXUS_DvbCiHandle handle, bool pcmciaMode);

/* Allow for misc. per-chip init and uninit routines */
extern NEXUS_Error NEXUS_DvbCi_P_InitChip(NEXUS_DvbCiHandle handle);
extern NEXUS_Error NEXUS_DvbCi_P_UninitChip(NEXUS_DvbCiHandle handle);

/* Hooks for basic card operations.  May be replaced with an extension. */

/* Returns true if a card is inserted, false otherwise. */
extern bool NEXUS_DvbCi_P_CardPresent(NEXUS_DvbCiHandle handle);

/* Enables the card interface if true, tristates the interface if false. */
extern NEXUS_Error NEXUS_DvbCi_P_SetupInterface(NEXUS_DvbCiHandle handle, bool pcmciaMode);
extern void NEXUS_DvbCi_P_SetVpp(NEXUS_DvbCiHandle handle, NEXUS_DvbCiVpp vpp);
extern void NEXUS_DvbCi_P_SetVcc(NEXUS_DvbCiHandle handle, NEXUS_DvbCiVcc vcc);
extern NEXUS_Error NEXUS_DvbCi_P_GetVoltageSense(NEXUS_DvbCiHandle handle, NEXUS_DvbCiPinState *pVs1State, NEXUS_DvbCiPinState *pVs2State);
extern void NEXUS_DvbCi_P_EnableReadyInterrupt(NEXUS_DvbCiHandle handle, bool enable, NEXUS_CallbackDesc *pCallback);
extern void NEXUS_DvbCi_P_EnableIreqInterrupt(NEXUS_DvbCiHandle handle, bool enable, NEXUS_CallbackDesc *pCallback);
extern void NEXUS_DvbCi_P_EnableCardDetectInterrupt(NEXUS_DvbCiHandle handle, bool enable, bool insertion, NEXUS_CallbackDesc *pCallback);
extern void NEXUS_DvbCi_P_WriteByte(NEXUS_DvbCiHandle handle, uint16_t address, uint8_t value);
extern uint8_t NEXUS_DvbCi_P_ReadByte(NEXUS_DvbCiHandle handle, uint16_t address);
extern void NEXUS_DvbCi_P_SetResetState(NEXUS_DvbCiHandle handle, NEXUS_DvbCiPinState state);

#endif /* #ifndef NEXUS_DVB_CI_MODULE_H__ */

